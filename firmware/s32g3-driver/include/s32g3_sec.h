/* SPDX-License-Identifier: GPL-2.0 */
/*
 * s32g3_sec.h - NXP S32G3 Security Engine Interface
 *
 * MTS-MB-3000 Mobile Backhaul S32G3 Driver
 */

#ifndef S32G3_SEC_H
#define S32G3_SEC_H

#include <linux/types.h>

#define S32G3_SEC_MAX_ALGOS 16
#define S32G3_SEC_MAX_KEYS 64
#define S32G3_SEC_MAX_SESSIONS 256
#define S32G3_SEC_KEY_NAME_LEN 32
#define S32G3_SEC_MAX_SPI 1024

enum s32g3_sec_algo {
    S32G3_SEC_ALGO_AES,
    S32G3_SEC_ALGO_3DES,
    S32G3_SEC_ALO_DES,
    S32G3_SEC_ALO_SHA1,
    S32G3_SEC_ALGO_SHA256,
    S32G3_SEC_ALO_SHA512,
    S32G3_SEC_ALO_MD5,
    S32G3_SEC_ALO_HMAC,
    S32G3_SEC_ALO_CRC,
    S32G3_SEC_ALO_NULL,
    S32G3_SEC_ALO_UNKNOWN
};

enum s32g3_sec_mode {
    S32G3_SEC_MODE_ECB,
    S32G3_SEC_MODE_CBC,
    S32G3_SEC_MODE_CFB,
    S32G3_SEC_MODE_OFB,
    S32G3_SEC_MODE_CTR,
    S32G3_SEC_MODE_GCM,
    S32G3_SEC_MODE_CCM,
    S32G3_SEC_MODE_XTS
};

struct s32g3_sec_key {
    uint32_t id;
    char name[S32G3_SEC_KEY_NAME_LEN];
    enum s32g3_sec_algo algo;
    enum s32g3_sec_mode mode;
    uint32_t key_len;
    uint8_t key[64];
    uint32_t iv_len;
    uint8_t iv[16];
    uint32_t active;
    uint64_t create_time;
    uint64_t last_use;
    uint32_t refcount;
};

struct s32g3_sec_session {
    uint32_t id;
    uint32_t key_id;
    uint32_t spi;
    enum s32g3_sec_algo algo;
    enum s32g3_sec_mode mode;
    uint32_t direction;  /* 0 = TX, 1 = RX */
    uint32_t active;
    uint64_t packets;
    uint64_t bytes;
    uint64_t errors;
    uint64_t create_time;
    uint64_t expire_time;
};

struct s32g3_sec_device {
    uint32_t id;
    char name[32];
    uint32_t num_keys;
    struct s32g3_sec_key *keys;
    uint32_t num_sessions;
    struct s32g3_sec_session *sessions;
    uint32_t num_spis;
    uint32_t *spi_table;
    void *priv;
    struct device *dev;
    struct mutex lock;
};

/* API */
int s32g3_sec_init(void);
void s32g3_sec_exit(void);
int s32g3_sec_key_create(uint32_t *key_id,
                          const char *name,
                          enum s32g3_sec_algo algo,
                          enum s32g3_sec_mode mode,
                          const uint8_t *key, uint32_t key_len);
int s32g3_sec_key_delete(uint32_t key_id);
int s32g3_sec_key_get(uint32_t key_id, struct s32g3_sec_key *key);
int s32g3_sec_session_create(uint32_t *sess_id,
                              uint32_t key_id, uint32_t spi,
                              enum s32g3_sec_algo algo,
                              enum s32g3_sec_mode mode,
                              uint32_t direction);
int s32g3_sec_session_delete(uint32_t sess_id);
int s32g3_sec_session_get(uint32_t sess_id, struct s32g3_sec_session *sess);
int s32g3_sec_encrypt(uint32_t sess_id,
                       const uint8_t *src, uint8_t *dst, uint32_t len);
int s32g3_sec_decrypt(uint32_t sess_id,
                       const uint8_t *src, uint8_t *dst, uint32_t len);
int s32g3_sec_digest(uint32_t key_id,
                      const uint8_t *data, uint32_t len,
                      uint8_t *digest, uint32_t *digest_len);

#endif /* S32G3_SEC_H */
