/*
 * rtl960x_crypto.c — Crypto operations for Realtek RTL960x GPON PHY
 *
 * MTS-RG-500 Residential Gateway — Драйвер криптографии GPON (OMCI encryption)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>

#include "rtl960x_gpon.h"

#define DRIVER_NAME "rtl960x-crypto"
#define DRIVER_VERSION "1.0.0"

/* ==================== GPON Crypto Registers ==================== */

#define RTL960X_CRYPTO_BASE_OFFSET    0x1E600000
#define RTL960X_CRYPTO_REG_SIZE       0x1000

/* Crypto control registers */
#define RTL960X_CRYPTO_CTRL           0x0000  /* Control register */
#define RTL960X_CRYPTO_STATUS         0x0004  /* Status register */
#define RTL960X_CRYPTO_KEY_LO         0x0008  /* Key low */
#define RTL960X_CRYPTO_KEY_HI         0x000C  /* Key high */
#define RTL960X_CRYPTO_IV_LO          0x0010  /* IV low */
#define RTL960X_CRYPTO_IV_HI          0x0014  /* IV high */
#define RTL960X_CRYPTO_SRC_LO         0x0018  /* Source low */
#define RTL960X_CRYPTO_SRC_HI         0x001C  /* Source high */
#define RTL960X_CRYPTO_DST_LO         0x0020  /* Dest low */
#define RTL960X_CRYPTO_DST_HI         0x0024  /* Dest high */
#define RTL960X_CRYPTO_LEN            0x0028  /* Data length */
#define RTL960X_CRYPTO_MODE           0x002C  /* Operation mode */
#define RTL960X_CRYPTO_INT_STATUS     0x0030  /* Interrupt status */
#define RTL960X_CRYPTO_INT_MASK       0x0034  /* Interrupt mask */

/* Crypto control bits */
#define RTL960X_CRYPTO_CTRL_START     (1 << 0)
#define RTL960X_CRYPTO_CTRL_EN        (1 << 1)
#define RTL960X_CRYPTO_CTRL_RESET     (1 << 2)

/* Crypto mode bits */
#define RTL960X_CRYPTO_MODE_AES       (1 << 0)
#define RTL960X_CRYPTO_MODE_AES128    (0 << 4)
#define RTL960X_CRYPTO_MODE_AES256    (2 << 4)
#define RTL960X_CRYPTO_MODE_ENC       (0 << 6)
#define RTL960X_CRYPTO_MODE_DEC       (1 << 6)
#define RTL960X_CRYPTO_MODE_CBC       (0 << 8)
#define RTL960X_CRYPTO_MODE_CFB       (1 << 8)

/* Status bits */
#define RTL960X_CRYPTO_STATUS_BUSY    (1 << 0)
#define RTL960X_CRYPTO_STATUS_DONE    (1 << 1)
#define RTL960X_CRYPTO_STATUS_ERR     (1 << 2)

/* ==================== Crypto Context ==================== */

#define RTL960X_CRYPTO_MAX_KEYS       4
#define RTL960X_CRYPTO_KEY_SIZE       32
#define RTL960X_CRYPTO_IV_SIZE        16

struct rtl960x_crypto_key {
    uint32_t id;
    uint8_t key[RTL960X_CRYPTO_KEY_SIZE];
    uint32_t key_len;
    bool valid;
};

struct rtl960x_crypto_ctx {
    void __iomem *base;
    struct mutex lock;
    uint32_t active_session;
    uint8_t mode;
    uint8_t key_size;
    struct rtl960x_crypto_key keys[RTL960X_CRYPTO_MAX_KEYS];
    uint8_t iv[RTL960X_CRYPTO_IV_SIZE];
    dma_addr_t dma_key;
    struct completion crypto_done;
    int last_error;
};

/* ==================== Hardware Operations ==================== */

static int rtl960x_crypto_write_reg(struct rtl960x_crypto_ctx *ctx,
                                     uint32_t reg, uint32_t value)
{
    writel(value, ctx->base + reg);
    return 0;
}

static uint32_t rtl960x_crypto_read_reg(struct rtl960x_crypto_ctx *ctx,
                                         uint32_t reg)
{
    return readl(ctx->base + reg);
}

static int rtl960x_crypto_wait_done(struct rtl960x_crypto_ctx *ctx,
                                     int timeout_ms)
{
    unsigned long timeout;

    init_completion(&ctx->crypto_done);
    timeout = msecs_to_jiffies(timeout_ms);

    while (timeout--) {
        uint32_t status = rtl960x_crypto_read_reg(ctx, RTL960X_CRYPTO_STATUS);
        if (status & RTL960X_CRYPTO_STATUS_DONE)
            return 0;
        if (status & RTL960X_CRYPTO_STATUS_ERR)
            return -EIO;
        msleep(1);
    }
    return -ETIMEDOUT;
}

static int rtl960x_crypto_load_key(struct rtl960x_crypto_ctx *ctx,
                                    uint32_t key_id,
                                    const uint8_t *key,
                                    uint32_t key_len)
{
    uint64_t key_addr;
    uint32_t key_lo, key_hi;

    if (!key || key_len == 0 || key_len > RTL960X_CRYPTO_KEY_SIZE)
        return -EINVAL;

    /* Map key to DMA */
    ctx->dma_key = dma_map_single(NULL, (void *)key, key_len, DMA_TO_DEVICE);
    if (dma_mapping_error(NULL, ctx->dma_key))
        return -ENOMEM;

    key_addr = (uint64_t)ctx->dma_key;
    key_lo = (uint32_t)(key_addr & 0xFFFFFFFF);
    key_hi = (uint32_t)(key_addr >> 32);

    /* Load key into hardware */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_KEY_LO, key_lo);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_KEY_HI, key_hi);

    /* Mark key slot as valid */
    ctx->keys[key_id].id = key_id;
    memcpy(ctx->keys[key_id].key, key, key_len);
    ctx->keys[key_id].key_len = key_len;
    ctx->keys[key_id].valid = true;

    return 0;
}

/* ==================== OMCI Encryption ==================== */

int rtl960x_crypto_init(struct rtl960x_crypto_ctx *ctx)
{
    if (!ctx)
        return -EINVAL;

    ctx->base = IOMEM(RTL960X_CRYPTO_BASE_OFFSET);
    mutex_init(&ctx->lock);

    /* Reset crypto engine */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL,
                             RTL960X_CRYPTO_CTRL_RESET);
    msleep(10);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL, 0);

    /* Enable crypto engine */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL,
                             RTL960X_CRYPTO_CTRL_EN);

    memset(ctx->iv, 0, RTL960X_CRYPTO_IV_SIZE);
    ctx->active_session = 0;
    ctx->last_error = 0;

    return 0;
}

void rtl960x_crypto_exit(struct rtl960x_crypto_ctx *ctx)
{
    if (!ctx)
        return;

    /* Disable crypto engine */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL, 0);

    if (ctx->dma_key)
        dma_unmap_single(NULL, ctx->dma_key,
                         RTL960X_CRYPTO_KEY_SIZE, DMA_TO_DEVICE);

    memset(ctx, 0, sizeof(*ctx));
}

int rtl960x_crypto_omci_encrypt(struct rtl960x_crypto_ctx *ctx,
                                 const uint8_t *plaintext,
                                 uint32_t plaintext_len,
                                 uint8_t *ciphertext,
                                 const uint8_t *key,
                                 uint32_t key_len)
{
    int ret;

    if (!ctx || !plaintext || !ciphertext || !key)
        return -EINVAL;

    if (key_len != 16 && key_len != 32)
        return -EINVAL;

    mutex_lock(&ctx->lock);

    /* Load key */
    ret = rtl960x_crypto_load_key(ctx, 0, key, key_len);
    if (ret)
        goto out;

    /* Set IV (zero for OMCI) */
    memset(ctx->iv, 0, RTL960X_CRYPTO_IV_SIZE);

    /* Set mode: AES + 128/256 + encrypt + CBC */
    uint32_t mode = RTL960X_CRYPTO_MODE_AES | RTL960X_CRYPTO_MODE_CBC |
                    RTL960X_CRYPTO_MODE_ENC;
    if (key_len == 16)
        mode |= RTL960X_CRYPTO_MODE_AES128;
    else
        mode |= RTL960X_CRYPTO_MODE_AES256;

    /* Configure source/dst addresses via DMA mapping */
    dma_addr_t src_dma = dma_map_single(NULL, (void *)plaintext,
                                          plaintext_len, DMA_TO_DEVICE);
    dma_addr_t dst_dma = dma_map_single(NULL, (void *)ciphertext,
                                          plaintext_len, DMA_FROM_DEVICE);

    if (dma_mapping_error(NULL, src_dma) || dma_mapping_error(NULL, dst_dma)) {
        ret = -ENOMEM;
        goto out;
    }

    /* Set addresses */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_SRC_LO, src_dma & 0xFFFFFFFF);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_SRC_HI, src_dma >> 32);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_DST_LO, dst_dma & 0xFFFFFFFF);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_DST_HI, dst_dma >> 32);

    /* Set data length */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_LEN, plaintext_len);

    /* Set IV */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_IV_LO,
                             *(uint32_t *)ctx->iv);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_IV_HI,
                             *(uint32_t *)(ctx->iv + 4));

    /* Clear interrupt status */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_INT_STATUS, 0xFFFFFFFF);

    /* Enable interrupts */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_INT_MASK,
                             RTL960X_CRYPTO_STATUS_DONE);

    /* Start operation */
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL,
                             RTL960X_CRYPTO_CTRL_START | RTL960X_CRYPTO_CTRL_EN);

    /* Wait for completion */
    ret = rtl960x_crypto_wait_done(ctx, 1000);

    /* Unmap DMA */
    dma_unmap_single(NULL, src_dma, plaintext_len, DMA_TO_DEVICE);
    dma_unmap_single(NULL, dst_dma, plaintext_len, DMA_FROM_DEVICE);

out:
    mutex_unlock(&ctx->lock);
    return ret;
}

int rtl960x_crypto_omci_decrypt(struct rtl960x_crypto_ctx *ctx,
                                  const uint8_t *ciphertext,
                                  uint32_t ciphertext_len,
                                  uint8_t *plaintext,
                                  const uint8_t *key,
                                  uint32_t key_len)
{
    int ret;

    if (!ctx || !ciphertext || !plaintext || !key)
        return -EINVAL;

    if (key_len != 16 && key_len != 32)
        return -EINVAL;

    mutex_lock(&ctx->lock);

    /* Load key */
    ret = rtl960x_crypto_load_key(ctx, 0, key, key_len);
    if (ret)
        goto out;

    /* Set IV */
    memset(ctx->iv, 0, RTL960X_CRYPTO_IV_SIZE);

    /* Set mode: AES + 128/256 + decrypt + CBC */
    uint32_t mode = RTL960X_CRYPTO_MODE_AES | RTL960X_CRYPTO_MODE_CBC |
                    RTL960X_CRYPTO_MODE_DEC;
    if (key_len == 16)
        mode |= RTL960X_CRYPTO_MODE_AES128;
    else
        mode |= RTL960X_CRYPTO_MODE_AES256;

    /* Configure addresses via DMA */
    dma_addr_t src_dma = dma_map_single(NULL, (void *)ciphertext,
                                          ciphertext_len, DMA_TO_DEVICE);
    dma_addr_t dst_dma = dma_map_single(NULL, (void *)plaintext,
                                          ciphertext_len, DMA_FROM_DEVICE);

    if (dma_mapping_error(NULL, src_dma) || dma_mapping_error(NULL, dst_dma)) {
        ret = -ENOMEM;
        goto out;
    }

    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_SRC_LO, src_dma & 0xFFFFFFFF);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_SRC_HI, src_dma >> 32);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_DST_LO, dst_dma & 0xFFFFFFFF);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_DST_HI, dst_dma >> 32);

    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_LEN, ciphertext_len);

    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_IV_LO,
                             *(uint32_t *)ctx->iv);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_IV_HI,
                             *(uint32_t *)(ctx->iv + 4));

    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_INT_STATUS, 0xFFFFFFFF);
    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_INT_MASK,
                             RTL960X_CRYPTO_STATUS_DONE);

    rtl960x_crypto_write_reg(ctx, RTL960X_CRYPTO_CTRL,
                             RTL960X_CRYPTO_CTRL_START | RTL960X_CRYPTO_CTRL_EN);

    ret = rtl960x_crypto_wait_done(ctx, 1000);

    dma_unmap_single(NULL, src_dma, ciphertext_len, DMA_TO_DEVICE);
    dma_unmap_single(NULL, dst_dma, ciphertext_len, DMA_FROM_DEVICE);

out:
    mutex_unlock(&ctx->lock);
    return ret;
}

/* ==================== GPON MACsec ==================== */

int rtl960x_crypto_macsec_init(struct rtl960x_crypto_ctx *ctx,
                                const uint8_t *sa_key,
                                uint32_t sa_idx)
{
    if (!ctx || !sa_key)
        return -EINVAL;

    mutex_lock(&ctx->lock);

    /* Load Secure Association key */
    int ret = rtl960x_crypto_load_key(ctx, sa_idx, sa_key, 16);
    if (ret)
        goto out;

    ctx->active_session++;

out:
    mutex_unlock(&ctx->lock);
    return ret;
}

int rtl960x_crypto_macsec_encrypt_frame(struct rtl960x_crypto_ctx *ctx,
                                          const uint8_t *frame,
                                          uint32_t frame_len,
                                          uint8_t *encrypted,
                                          uint32_t sa_idx)
{
    if (!ctx || !frame || !encrypted)
        return -EINVAL;

    /* MACsec encryption placeholder */
    return rtl960x_crypto_omci_encrypt(ctx, frame, frame_len, encrypted, NULL, 0);
}

int rtl960x_crypto_macsec_decrypt_frame(struct rtl960x_crypto_ctx *ctx,
                                          const uint8_t *encrypted,
                                          uint32_t enc_len,
                                          uint8_t *frame,
                                          uint32_t sa_idx)
{
    if (!ctx || !encrypted || !frame)
        return -EINVAL;

    /* MACsec decryption placeholder */
    return rtl960x_crypto_omci_decrypt(ctx, encrypted, enc_len, frame, NULL, 0);
}

/* ==================== TR-069 Encryption ==================== */

int rtl960x_crypto_tr069_encrypt(struct rtl960x_crypto_ctx *ctx,
                                    const uint8_t *data,
                                    uint32_t data_len,
                                    uint8_t *encrypted,
                                    const uint8_t *key,
                                    uint32_t key_len)
{
    /* TR-069 uses AES-CBC for SOAP encryption */
    return rtl960x_crypto_omci_encrypt(ctx, data, data_len,
                                         encrypted, key, key_len);
}

int rtl960x_crypto_tr069_decrypt(struct rtl960x_crypto_ctx *ctx,
                                    const uint8_t *encrypted,
                                    uint32_t enc_len,
                                    uint8_t *data,
                                    const uint8_t *key,
                                    uint32_t key_len)
{
    return rtl960x_crypto_omci_decrypt(ctx, encrypted, enc_len,
                                         data, key, key_len);
}

/* ==================== RNG ==================== */

int rtl960x_crypto_rng_get_random(struct rtl960x_crypto_ctx *ctx,
                                    uint8_t *buf,
                                    uint32_t len)
{
    if (!ctx || !buf || len == 0)
        return -EINVAL;

    /* Read from hardware RNG registers */
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t rng_val = rtl960x_crypto_read_reg(ctx, 0x0040);
        uint32_t offset = i % 4;
        buf[i] = (rng_val >> (offset * 8)) & 0xFF;
    }

    return 0;
}

/* ==================== Crypto Status ==================== */

int rtl960x_crypto_get_status(struct rtl960x_crypto_ctx *ctx,
                               uint32_t *active_sessions,
                               uint32_t *errors,
                               uint64_t *ops_completed)
{
    if (!ctx)
        return -EINVAL;

    uint32_t status = rtl960x_crypto_read_reg(ctx, RTL960X_CRYPTO_STATUS);

    if (active_sessions)
        *active_sessions = ctx->active_session;
    if (errors)
        *errors = (status & RTL960X_CRYPTO_STATUS_ERR) ? 1 : 0;
    if (ops_completed)
        *ops_completed = 0;

    return 0;
}

/* ==================== Module ==================== */

static int __init rtl960x_crypto_init_module(void)
{
    pr_info("RTL960x Crypto driver loaded (version %s)\n", DRIVER_VERSION);
    return 0;
}

static void __exit rtl960x_crypto_exit_module(void)
{
    pr_info("RTL960x Crypto driver unloaded\n");
}

module_init(rtl960x_crypto_init_module);
module_exit(rtl960x_crypto_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Team");
MODULE_DESCRIPTION("Realtek RTL960x GPON Crypto Driver (AES-OMCI/MACsec/TR-069)");
MODULE_VERSION(DRIVER_VERSION);
