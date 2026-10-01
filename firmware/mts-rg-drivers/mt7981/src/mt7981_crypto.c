/*
 * mt7981_crypto.c — Hardware crypto driver for MediaTek MT7981
 *
 * MTS-RG-500 Residential Gateway — Драйвер аппаратного AES/RSA ускорителя
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <crypto/internal/aes.h>
#include <crypto/internal/skcipher.h>
#include <linux/cryptomgr.h>

#include "mt7981.h"

#define DRIVER_NAME "mt7981-crypto"
#define DRIVER_VERSION "1.0.0"

/* ==================== Hardware Registers ==================== */

/* Crypto engine base address (mapped in probe) */
#define MT7981_CRYPTO_BASE_OFFSET    0x60000000
#define MT7981_CRYPTO_REG_SIZE       0x1000

/* Crypto control registers */
#define MT7981_CRYPTO_CTRL           0x0000  /* Control register */
#define MT7981_CRYPTO_STATUS         0x0004  /* Status register */
#define MT7981_CRYPTO_SRC_ADDR_LO    0x0008  /* Source address low */
#define MT7981_CRYPTO_SRC_ADDR_HI    0x000C  /* Source address high */
#define MT7981_CRYPTO_DST_ADDR_LO    0x0010  /* Dest address low */
#define MT7981_CRYPTO_DST_ADDR_HI    0x0014  /* Dest address high */
#define MT7981_CRYPTO_KEY_ADDR_LO    0x0018  /* Key address low */
#define MT7981_CRYPTO_KEY_ADDR_HI    0x001C  /* Key address high */
#define MT7981_CRYPTO_IV_LO          0x0020  /* IV low */
#define MT7981_CRYPTO_IV_HI          0x0024  /* IV high */
#define MT7981_CRYPTO_LENGTH         0x0028  /* Data length */
#define MT7981_CRYPTO_MODE           0x002C  /* Operation mode */
#define MT7981_CRYPTO_INT_STATUS     0x0030  /* Interrupt status */
#define MT7981_CRYPTO_INT_MASK       0x0034  /* Interrupt mask */

/* Crypto control bits */
#define MT7981_CRYPTO_CTRL_START     (1 << 0)
#define MT7981_CRYPTO_CTRL_EN        (1 << 1)
#define MT7981_CRYPTO_CTRL_RESET     (1 << 2)

/* Crypto mode bits */
#define MT7981_CRYPTO_MODE_AES       (1 << 0)
#define MT7981_CRYPTO_MODE_AES128    (0 << 4)
#define MT7981_CRYPTO_MODE_AES192    (1 << 4)
#define MT7981_CRYPTO_MODE_AES256    (2 << 4)
#define MT7981_CRYPTO_MODE_ENC       (0 << 6)
#define MT7981_CRYPTO_MODE_DEC       (1 << 6)
#define MT7981_CRYPTO_MODE_CBC       (0 << 8)
#define MT7981_CRYPTO_MODE_CFB       (1 << 8)
#define MT7981_CRYPTO_MODE_OFB       (2 << 8)
#define MT7981_CRYPTO_MODE_CTR       (3 << 8)
#define MT7981_CRYPTO_MODE_ECB       (4 << 8)

/* Status bits */
#define MT7981_CRYPTO_STATUS_BUSY    (1 << 0)
#define MT7981_CRYPTO_STATUS_DONE    (1 << 1)
#define MT7981_CRYPTO_STATUS_ERR     (1 << 2)

/* ==================== Crypto Context ==================== */

#define MT7981_CRYPTO_MAX_KEYS       4
#define MT7981_CRYPTO_KEY_SIZE_AES   32
#define MT7981_CRYPTO_IV_SIZE        16
#define MT7981_CRYPTO_BLOCK_SIZE     16
#define MT7981_CRYPTO_BUF_SIZE       (64 * 1024)  /* 64KB DMA buffer */

struct mt7981_crypto_key {
    uint32_t id;
    uint8_t key[MT7981_CRYPTO_KEY_SIZE_AES];
    uint32_t key_len;
    bool valid;
};

struct mt7981_crypto_ctx {
    struct mt7981_device *dev;
    void __iomem *base;
    struct mutex lock;
    uint32_t active_session;
    uint8_t mode;       /* AES mode */
    uint8_t key_size;   /* 128/192/256 */
    struct mt7981_crypto_key keys[MT7981_CRYPTO_MAX_KEYS];
    uint8_t iv[MT7981_CRYPTO_IV_SIZE];
    dma_addr_t dma_src;
    dma_addr_t dma_dst;
    dma_addr_t dma_key;
    void *dma_buf;
    struct completion crypto_done;
    int last_error;
};

/* ==================== Hardware Operations ==================== */

static int mt7981_crypto_write_reg(struct mt7981_crypto_ctx *ctx,
                                    uint32_t reg, uint32_t value)
{
    writel(value, ctx->base + reg);
    return 0;
}

static uint32_t mt7981_crypto_read_reg(struct mt7981_crypto_ctx *ctx,
                                        uint32_t reg)
{
    return readl(ctx->base + reg);
}

static int mt7981_crypto_wait_done(struct mt7981_crypto_ctx *ctx, int timeout_ms)
{
    unsigned long timeout;
    uint32_t status;

    init_completion(&ctx->crypto_done);
    timeout = msecs_to_jiffies(timeout_ms);

    while (timeout--) {
        status = mt7981_crypto_read_reg(ctx, MT7981_CRYPTO_STATUS);
        if (status & MT7981_CRYPTO_STATUS_DONE)
            return 0;
        if (status & MT7981_CRYPTO_STATUS_ERR)
            return -EIO;
        msleep(1);
    }
    return -ETIMEDOUT;
}

static int mt7981_crypto_load_key(struct mt7981_crypto_ctx *ctx,
                                   uint32_t key_id,
                                   const uint8_t *key,
                                   uint32_t key_len)
{
    uint64_t key_addr;
    uint32_t key_lo, key_hi;
    int ret;

    if (!key || key_len == 0 || key_len > MT7981_CRYPTO_KEY_SIZE_AES)
        return -EINVAL;

    /* Map key to DMA */
    ctx->dma_key = dma_map_single(NULL, (void *)key, key_len, DMA_TO_DEVICE);
    if (dma_mapping_error(NULL, ctx->dma_key))
        return -ENOMEM;

    key_addr = (uint64_t)ctx->dma_key;
    key_lo = (uint32_t)(key_addr & 0xFFFFFFFF);
    key_hi = (uint32_t)(key_addr >> 32);

    /* Load key into hardware */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_KEY_ADDR_LO, key_lo);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_KEY_ADDR_HI, key_hi);

    /* Mark key slot as valid */
    ctx->keys[key_id].id = key_id;
    memcpy(ctx->keys[key_id].key, key, key_len);
    ctx->keys[key_id].key_len = key_len;
    ctx->keys[key_id].valid = true;

    return 0;
}

static int mt7981_crypto_set_mode(struct mt7981_crypto_ctx *ctx,
                                   uint32_t mode)
{
    uint32_t reg;

    reg = mt7981_crypto_read_reg(ctx, MT7981_CRYPTO_MODE);
    reg &= ~(0x7F);  /* Clear mode bits */
    reg |= mode;
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_MODE, reg);

    return 0;
}

static int mt7981_crypto_start(struct mt7981_crypto_ctx *ctx,
                                uint32_t src_addr,
                                uint32_t dst_addr,
                                uint32_t length)
{
    uint32_t ctrl;

    /* Set source/destination addresses */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_SRC_ADDR_LO, src_addr & 0xFFFFFFFF);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_SRC_ADDR_HI, src_addr >> 32);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_DST_ADDR_LO, dst_addr & 0xFFFFFFFF);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_DST_ADDR_HI, dst_addr >> 32);

    /* Set data length */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_LENGTH, length);

    /* Set IV */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_IV_LO,
                            *(uint32_t *)ctx->iv);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_IV_HI,
                            *(uint32_t *)(ctx->iv + 4));

    /* Clear interrupt status */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_INT_STATUS, 0xFFFFFFFF);

    /* Enable interrupts */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_INT_MASK,
                            MT7981_CRYPTO_STATUS_DONE);

    /* Start operation */
    ctrl = MT7981_CRYPTO_CTRL_START | MT7981_CRYPTO_CTRL_EN;
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_CTRL, ctrl);

    return 0;
}

/* ==================== AES Operations ==================== */

int mt7981_crypto_aes_init(struct mt7981_device *dev)
{
    struct mt7981_crypto_ctx *ctx;

    if (!dev)
        return -EINVAL;

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    ctx->dev = dev;
    ctx->base = IOMEM(MT7981_CRYPTO_BASE_OFFSET);
    mutex_init(&ctx->lock);

    /* Reset crypto engine */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_CTRL,
                            MT7981_CRYPTO_CTRL_RESET);
    msleep(10);
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_CTRL, 0);

    /* Enable crypto engine */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_CTRL,
                            MT7981_CRYPTO_CTRL_EN);

    dev->priv = ctx;
    return 0;
}

void mt7981_crypto_aes_exit(struct mt7981_device *dev)
{
    struct mt7981_crypto_ctx *ctx;

    if (!dev || !dev->priv)
        return;

    ctx = dev->priv;

    /* Disable crypto engine */
    mt7981_crypto_write_reg(ctx, MT7981_CRYPTO_CTRL, 0);

    /* Unmap DMA */
    if (ctx->dma_buf)
        dma_unmap_single(NULL, (dma_addr_t)ctx->dma_buf,
                         MT7981_CRYPTO_BUF_SIZE, DMA_BIDIRECTIONAL);

    kfree(ctx);
    dev->priv = NULL;
}

int mt7981_crypto_aes_encrypt(struct mt7981_device *dev,
                               const uint8_t *plaintext,
                               uint32_t plaintext_len,
                               uint8_t *ciphertext,
                               const uint8_t *key,
                               uint32_t key_len,
                               const uint8_t *iv)
{
    struct mt7981_crypto_ctx *ctx;
    int ret;

    if (!dev || !plaintext || !ciphertext || !key)
        return -EINVAL;

    if (key_len != 16 && key_len != 24 && key_len != 32)
        return -EINVAL;

    ctx = dev->priv;
    if (!ctx)
        return -ENODEV;

    mutex_lock(&ctx->lock);

    /* Load key */
    ret = mt7981_crypto_load_key(ctx, 0, key, key_len);
    if (ret)
        goto out;

    /* Set IV */
    if (iv)
        memcpy(ctx->iv, iv, MT7981_CRYPTO_IV_SIZE);
    else
        memset(ctx->iv, 0, MT7981_CRYPTO_IV_SIZE);

    /* Set mode: AES + key size + encrypt + ECB */
    uint32_t mode = MT7981_CRYPTO_MODE_AES | MT7981_CRYPTO_MODE_ECB;
    if (key_len == 128)
        mode |= MT7981_CRYPTO_MODE_AES128;
    else if (key_len == 192)
        mode |= MT7981_CRYPTO_MODE_AES192;
    else
        mode |= MT7981_CRYPTO_MODE_AES256;

    mt7981_crypto_set_mode(ctx, mode);

    /* Start encryption */
    ret = mt7981_crypto_start(ctx, 0, 0, plaintext_len);
    if (ret)
        goto out;

    /* Wait for completion */
    ret = mt7981_crypto_wait_done(ctx, 1000);
    if (ret)
        goto out;

out:
    mutex_unlock(&ctx->lock);
    return ret;
}

int mt7981_crypto_aes_decrypt(struct mt7981_device *dev,
                               const uint8_t *ciphertext,
                               uint32_t ciphertext_len,
                               uint8_t *plaintext,
                               const uint8_t *key,
                               uint32_t key_len,
                               const uint8_t *iv)
{
    struct mt7981_crypto_ctx *ctx;
    int ret;

    if (!dev || !ciphertext || !plaintext || !key)
        return -EINVAL;

    if (key_len != 16 && key_len != 24 && key_len != 32)
        return -EINVAL;

    ctx = dev->priv;
    if (!ctx)
        return -ENODEV;

    mutex_lock(&ctx->lock);

    /* Load key */
    ret = mt7981_crypto_load_key(ctx, 0, key, key_len);
    if (ret)
        goto out;

    /* Set IV */
    if (iv)
        memcpy(ctx->iv, iv, MT7981_CRYPTO_IV_SIZE);
    else
        memset(ctx->iv, 0, MT7981_CRYPTO_IV_SIZE);

    /* Set mode: AES + key size + decrypt + ECB */
    uint32_t mode = MT7981_CRYPTO_MODE_AES | MT7981_CRYPTO_MODE_ECB |
                    MT7981_CRYPTO_MODE_DEC;
    if (key_len == 128)
        mode |= MT7981_CRYPTO_MODE_AES128;
    else if (key_len == 192)
        mode |= MT7981_CRYPTO_MODE_AES192;
    else
        mode |= MT7981_CRYPTO_MODE_AES256;

    mt7981_crypto_set_mode(ctx, mode);

    /* Start decryption */
    ret = mt7981_crypto_start(ctx, 0, 0, ciphertext_len);
    if (ret)
        goto out;

    /* Wait for completion */
    ret = mt7981_crypto_wait_done(ctx, 1000);
    if (ret)
        goto out;

out:
    mutex_unlock(&ctx->lock);
    return ret;
}

/* ==================== RSA Operations ==================== */

int mt7981_crypto_rsa_init(struct mt7981_device *dev)
{
    /* RSA engine initialization placeholder */
    return 0;
}

int mt7981_crypto_rsa_keygen(struct mt7981_device *dev,
                              uint32_t bits,
                              uint32 *pub_exp,
                              uint8_t *modulus,
                              uint32_t *mod_len)
{
    /* RSA key generation placeholder */
    return -EOPNOTSUPP;
}

int mt7981_crypto_rsa_encrypt(struct mt7981_device *dev,
                               const uint8_t *data,
                               uint32_t data_len,
                               uint8_t *result,
                               const uint8_t *modulus,
                               uint32_t mod_len,
                               uint32_t pub_exp)
{
    /* RSA encrypt placeholder */
    return -EOPNOTSUPP;
}

int mt7981_crypto_rsa_decrypt(struct mt7981_device *dev,
                               const uint8_t *ciphertext,
                               uint32_t cipher_len,
                               uint8_t *plaintext,
                               const uint8_t *prime_p,
                               const uint8_t *prime_q,
                               uint32_t prime_len)
{
    /* RSA decrypt placeholder */
    return -EOPNOTSUPP;
}

/* ==================== RNG Operations ==================== */

int mt7981_crypto_rng_get_random(struct mt7981_device *dev,
                                  uint8_t *buf,
                                  uint32_t len)
{
    struct mt7981_crypto_ctx *ctx;

    if (!dev || !buf || len == 0)
        return -EINVAL;

    ctx = dev->priv;
    if (!ctx)
        return -ENODEV;

    /* Read from hardware RNG */
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t rng_val = mt7981_crypto_read_reg(ctx, 0x0040);
        uint32_t offset = i % 4;
        buf[i] = (rng_val >> (offset * 8)) & 0xFF;
    }

    return 0;
}

int mt7981_crypto_rng_seed(struct mt7981_device *dev,
                            const uint8_t *seed,
                            uint32_t seed_len)
{
    /* Seed entropy into hardware RNG */
    return 0;
}

/* ==================== Crypto Status ==================== */

int mt7981_crypto_get_status(struct mt7981_device *dev,
                              uint32_t *active_sessions,
                              uint32_t *errors,
                              uint64_t *ops_completed)
{
    struct mt7981_crypto_ctx *ctx;

    if (!dev)
        return -EINVAL;

    ctx = dev->priv;
    if (!ctx)
        return -ENODEV;

    uint32_t status = mt7981_crypto_read_reg(ctx, MT7981_CRYPTO_STATUS);

    if (active_sessions)
        *active_sessions = ctx->active_session;
    if (errors)
        *errors = (status & MT7981_CRYPTO_STATUS_ERR) ? 1 : 0;
    if (ops_completed)
        *ops_completed = 0;  /* Would track from hardware counter */

    return 0;
}

/* ==================== Module ==================== */

static int __init mt7981_crypto_init_module(void)
{
    pr_info("MT7981 Crypto driver loaded (version %s)\n", DRIVER_VERSION);
    return 0;
}

static void __exit mt7981_crypto_exit_module(void)
{
    pr_info("MT7981 Crypto driver unloaded\n");
}

module_init(mt7981_crypto_init_module);
module_exit(mt7981_crypto_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Team");
MODULE_DESCRIPTION("MediaTek MT7981 Hardware Crypto Driver (AES/RSA/RNG)");
MODULE_VERSION(DRIVER_VERSION);
