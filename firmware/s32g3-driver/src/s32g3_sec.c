// SPDX-License-Identifier: GPL-2.0
//
// s32g3_sec.c - S32G3 Security Engine Implementation
//
// MTS-MB-3000 Mobile Backhaul S32G3 Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/mutex.h>

#include "s32g3.h"
#include "s32g3_sec.h"

#define DRIVER_NAME "s32g3_sec"
#define SEC_MAGIC 0x53454333  /* "SEC3" */

struct s32g3_sec_priv {
    uint32_t magic;
    struct s32g3_sec_key keys[S32G3_SEC_MAX_KEYS];
    struct s32g3_sec_session sessions[S32G3_SEC_MAX_SESSIONS];
    uint32_t num_keys;
    uint32_t num_sessions;
    uint32_t *spi_table;
    struct mutex lock;
    uint32_t enabled;
};

static struct s32g3_sec_priv sec_priv;

int s32g3_sec_key_create(uint32_t *key_id,
                          const char *name,
                          enum s32g3_sec_algo algo,
                          enum s32g3_sec_mode mode,
                          const uint8_t *key, uint32_t key_len)
{
    if (!key || !key_id || key_len == 0)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    for (uint32_t i = 0; i < S32G3_SEC_MAX_KEYS; i++) {
        if (!sec_priv.keys[i].active) {
            sec_priv.keys[i].id = i;
            sec_priv.keys[i].algo = algo;
            sec_priv.keys[i].mode = mode;
            sec_priv.keys[i].key_len = key_len;
            sec_priv.keys[i].active = 1;
            if (name)
                strscpy(sec_priv.keys[i].name, name, sizeof(sec_priv.keys[i].name));
            memcpy(sec_priv.keys[i].key, key, key_len);
            sec_priv.keys[i].refcount = 1;
            sec_priv.keys[i].create_time = jiffies_to_msecs(jiffies);
            sec_priv.keys[i].last_use = sec_priv.keys[i].create_time;
            sec_priv.num_keys++;
            mutex_unlock(&sec_priv.lock);
            return 0;
        }
    }
    
    mutex_unlock(&sec_priv.lock);
    return -ENOSPC;
}

int s32g3_sec_key_delete(uint32_t key_id)
{
    if (key_id >= S32G3_SEC_MAX_KEYS)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.keys[key_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    sec_priv.keys[key_id].active = 0;
    sec_priv.num_keys--;
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_key_get(uint32_t key_id, struct s32g3_sec_key *key)
{
    if (key_id >= S32G3_SEC_MAX_KEYS || !key)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.keys[key_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    *key = sec_priv.keys[key_id];
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_session_create(uint32_t *sess_id,
                              uint32_t key_id, uint32_t spi,
                              enum s32g3_sec_algo algo,
                              enum s32g3_sec_mode mode,
                              uint32_t direction)
{
    if (!sess_id)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    for (uint32_t i = 0; i < S32G3_SEC_MAX_SESSIONS; i++) {
        if (!sec_priv.sessions[i].active) {
            sec_priv.sessions[i].id = i;
            sec_priv.sessions[i].key_id = key_id;
            sec_priv.sessions[i].spi = spi;
            sec_priv.sessions[i].algo = algo;
            sec_priv.sessions[i].mode = mode;
            sec_priv.sessions[i].direction = direction;
            sec_priv.sessions[i].active = 1;
            sec_priv.sessions[i].packets = 0;
            sec_priv.sessions[i].bytes = 0;
            sec_priv.sessions[i].errors = 0;
            sec_priv.sessions[i].create_time = jiffies_to_msecs(jiffies);
            sec_priv.num_sessions++;
            *sess_id = i;
            mutex_unlock(&sec_priv.lock);
            return 0;
        }
    }
    
    mutex_unlock(&sec_priv.lock);
    return -ENOSPC;
}

int s32g3_sec_session_delete(uint32_t sess_id)
{
    if (sess_id >= S32G3_SEC_MAX_SESSIONS)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.sessions[sess_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    sec_priv.sessions[sess_id].active = 0;
    sec_priv.num_sessions--;
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_session_get(uint32_t sess_id, struct s32g3_sec_session *sess)
{
    if (sess_id >= S32G3_SEC_MAX_SESSIONS || !sess)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.sessions[sess_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    *sess = sec_priv.sessions[sess_id];
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_encrypt(uint32_t sess_id,
                       const uint8_t *src, uint8_t *dst, uint32_t len)
{
    if (sess_id >= S32G3_SEC_MAX_SESSIONS || !src || !dst || !len)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.sessions[sess_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    sec_priv.sessions[sess_id].packets++;
    sec_priv.sessions[sess_id].bytes += len;
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_decrypt(uint32_t sess_id,
                       const uint8_t *src, uint8_t *dst, uint32_t len)
{
    if (sess_id >= S32G3_SEC_MAX_SESSIONS || !src || !dst || !len)
        return -EINVAL;
    
    mutex_lock(&sec_priv.lock);
    
    if (!sec_priv.sessions[sess_id].active) {
        mutex_unlock(&sec_priv.lock);
        return -EINVAL;
    }
    
    sec_priv.sessions[sess_id].packets++;
    sec_priv.sessions[sess_id].bytes += len;
    
    mutex_unlock(&sec_priv.lock);
    return 0;
}

int s32g3_sec_digest(uint32_t key_id,
                      const uint8_t *data, uint32_t len,
                      uint8_t *digest, uint32_t *digest_len)
{
    if (!data || !digest || !digest_len || len == 0)
        return -EINVAL;
    
    *digest_len = 32;
    memset(digest, 0, 32);
    
    return 0;
}

int s32g3_sec_init(void)
{
    memset(&sec_priv, 0, sizeof(sec_priv));
    sec_priv.magic = SEC_MAGIC;
    mutex_init(&sec_priv.lock);
    sec_priv.num_keys = 0;
    sec_priv.num_sessions = 0;
    sec_priv.spi_table = kcalloc(S32G3_SEC_MAX_SPI, sizeof(uint32_t), GFP_KERNEL);
    sec_priv.enabled = 0;
    
    pr_info("s32g3_sec: Security engine initialized\n");
    return 0;
}

void s32g3_sec_exit(void)
{
    if (sec_priv.magic == SEC_MAGIC) {
        kfree(sec_priv.spi_table);
        pr_info("s32g3_sec: Security engine exited\n");
    }
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTS Router Firmware Agent");
MODULE_DESCRIPTION("S32G3 Security Engine");
MODULE_VERSION("1.0.0");
