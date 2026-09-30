/* SPDX-License-Identifier: GPL-2.0 */
/*
 * tofino2_ctrl.h - Control Plane Interface (Counters, Meters, DMA)
 *
 * MTS-CR-9000 Core Router Tofino 2 Driver
 *
 * Copyright (c) 2024 MTS Router Project
 */

#ifndef TOFINO2_CTRL_H
#define TOFINO2_CTRL_H

#include <linux/types.h>
#include <linux/dmaengine.h>
#include <linux/kref.h>

/* ============================================================================ */
/* Constants                                                                    */
/* ============================================================================ */

#define TOFINO2_CTRL_MAX_COUNTERS    8192
#define TOFINO2_CTRL_MAX_METERS       4096
#define TOFINO2_CTRL_MAX_DMA_CHANNELS 64
#define TOFINO2_CTRL_METER_NAME_LEN   32
#define TOFINO2_CTRL_COUNTER_NAME_LEN 32
#define TOFINO2_CTRL_DMA_BUF_MAX      (1024 * 1024)  /* 1MB max DMA buf */
#define TOFINO2_CTRL_DMA_MIN          PAGE_SIZE
#define TOFINO2_CTRL_DMA_DEFAULT      (PAGE_SIZE * 16)
#define TOFINO2_CTRL_DIGEST_SIZE      64
#define TOFINO2_CTRL_MAX_DIGESTS      256

/* ============================================================================ */
/* Enums                                                                        */
/* ============================================================================ */

/**
 * enum tofino2_ctrl_counter_mode - Counter operation mode
 */
enum tofino2_ctrl_counter_mode {
    TOFINO2_CTRL_COUNTER_64BIT    = 0,
    TOFINO2_CTRL_COUNTER_128BIT   = 1,
    TOFINO2_CTRL_COUNTER_PORT     = 2,
    TOFINO2_CTRL_COUNTER_SYSTEM   = 3,
    TOFINO2_CTRL_COUNTER_DIGEST   = 4,
    TOFINO2_CTRL_COUNTER_HASH     = 5
};

/**
 * enum tofino2_ctrl_meter_mode - Meter operation mode
 */
enum tofino2_ctrl_meter_mode {
    TOFINO2_CTRL_METER_SINGLE_RATE_TWO_BUCKET  = 0,
    TOFINO2_CTRL_METER_DOUBLE_RATE_TWO_BUCKET  = 1,
    TOFINO2_CTRL_METER_SINGLE_RATE_THREE_BUCKET = 2,
    TOFINO2_CTRL_METER_DOUBLE_RATE_THREE_BUCKET = 3
};

/**
 * enum tofino2_ctrl_dma_direction - DMA transfer direction
 */
enum tofino2_ctrl_dma_direction {
    TOFINO2_CTRL_DMA_NONE     = 0,
    TOFINO2_CTRL_DMA_TO_DEVICE = 1,
    TOFINO2_CTRL_DMA_FROM_DEVICE = 2,
    TOFINO2_CTRL_DMA_BIDIRECTIONAL = 3
};

/**
 * enum tofino2_ctrl_dma_status - DMA channel status
 */
enum tofino2_ctrl_dma_status {
    TOFINO2_CTRL_DMA_IDLE      = 0,
    TOFINO2_CTRL_DMA_QUEUED    = 1,
    TOFINO2_CTRL_DMA_RUNNING   = 2,
    TOFINO2_CTRL_DMA_COMPLETED = 3,
    TOFINO2_CTRL_DMA_ERROR     = 4,
    TOFINO2_CTRL_DMA_ABORTED   = 5
};

/**
 * enum tofino2_ctrl_digest_type - Digest computation type
 */
enum tofino2_ctrl_digest_type {
    TOFINO2_CTRL_DIGEST_NONE   = 0,
    TOFINO2_CTRL_DIGEST_CRC8   = 1,
    TOFINO2_CTRL_DIGEST_CRC16  = 2,
    TOFINO2_CTRL_DIGEST_CRC32  = 3,
    TOFINO2_CTRL_DIGEST_IP     = 4,
    TOFINO2_CTRL_DIGEST_UDP    = 5,
    TOFINO2_CTRL_DIGEST_CUSTOM = 6
};

/* ============================================================================ */
/* Structures                                                                   */
/* ============================================================================ */

/**
 * struct tofino2_ctrl_counter - Counter control block
 */
struct tofino2_ctrl_counter {
    uint32_t id;                            /* Counter ID */
    char name[TOFINO2_CTRL_COUNTER_NAME_LEN]; /* Counter name */
    enum tofino2_ctrl_counter_mode mode;    /* Counter mode */
    uint64_t value;                         /* Current counter value */
    uint64_t base_value;                    /* Base/reset value */
    uint64_t max_value;                     /* Max value since last read */
    uint64_t min_value;                     /* Min value since last read */
    uint64_t timestamp;                     /* Last update timestamp */
    uint64_t last_update;                   /* Last value change time */
    uint32_t width;                         /* Counter width (64/128) */
    uint32_t enabled;                       /* Counter enabled */
    uint32_t reset;                         /* Reset pending */
    uint32_t allocated;                     /* Allocated flag */
    uint32_t refcount;                      /* Reference count */
    struct kref kref;                       /* Kernel reference */
    struct list_head list;                  /* List node */
    uint8_t metadata[64];                   /* User metadata */
};

/**
 * struct tofino2_ctrl_meter - Meter control block
 */
struct tofino2_ctrl_meter {
    uint32_t id;                            /* Meter ID */
    char name[TOFINO2_CTRL_METER_NAME_LEN]; /* Meter name */
    enum tofino2_ctrl_meter_mode mode;      /* Meter mode */
    uint32_t cir;                           /* Committed info rate (bps) */
    uint32_t pir;                           /* Peak info rate (bps) */
    uint32_t cbs;                           /* Committed burst size (bytes) */
    uint32_t pbs;                           /* Peak burst size (bytes) */
    uint32_t ebs;                           /* Excess burst size (bytes) */
    uint32_t current_credits;               /* Current token credits */
    uint32_t last_credits;                  /* Last credits value */
    uint64_t last_update;                   /* Last update timestamp */
    uint64_t packet_count;                  /* Packets processed */
    uint64_t green_packets;                 /* Green packets */
    uint64_t yellow_packets;                /* Yellow packets */
    uint64_t red_packets;                   /* Red packets */
    uint64_t green_bytes;                   /* Green bytes */
    uint64_t yellow_bytes;                  /* Yellow bytes */
    uint64_t red_bytes;                     /* Red bytes */
    uint32_t enabled;                       /* Meter enabled */
    uint32_t allocated;                     /* Allocated flag */
    uint32_t refcount;                      /* Reference count */
    struct kref kref;                       /* Kernel reference */
    struct list_head list;                  /* List node */
    uint8_t metadata[64];                   /* User metadata */
};

/**
 * struct tofino2_ctrl_dma - DMA channel control block
 */
struct tofino2_ctrl_dma {
    uint32_t id;                            /* Channel ID */
    enum tofino2_ctrl_dma_direction dir;    /* DMA direction */
    enum tofino2_ctrl_dma_status status;    /* Channel status */
    uint64_t src_addr;                      /* Source address (DMA coherent) */
    uint64_t dst_addr;                      /* Destination address */
    uint32_t length;                        /* Transfer length */
    uint32_t completed;                     /* Bytes completed */
    uint32_t max_length;                    /* Max transfer length */
    uint32_t irq;                           /* Associated IRQ */
    uint32_t irq_count;                     /* IRQ trigger count */
    uint32_t error_count;                   /* Error count */
    uint8_t *virt_addr;                     /* Virtual address */
    dma_addr_t dma_addr;                    /* DMA/bus address */
    uint32_t dma_len;                       /* DMA mapping length */
    struct dma_chan *dma_chan;              /* Linux DMA channel */
    struct dma_async_tx_descriptor *desc;   /* DMA descriptor */
    struct completion completion;           /* DMA completion event */
    uint32_t enabled;                       /* Channel enabled */
    uint32_t allocated;                     /* Channel allocated */
    uint32_t mode;                          /* DMA mode */
    uint32_t burst_size;                    /* Burst size */
    uint32_t src_width;                     /* Source width */
    uint32_t dst_width;                     /* Dest width */
    uint32_t src_maxburst;                  /* Source max burst */
    uint32_t dst_maxburst;                  /* Dest max burst */
    uint8_t metadata[64];                   /* User metadata */
};

/**
 * struct tofino2_ctrl_digest - Digest computation state
 */
struct tofino2_ctrl_digest {
    uint32_t id;                            /* Digest ID */
    char name[32];                          /* Digest name */
    enum tofino2_ctrl_digest_type type;     /* Digest type */
    uint8_t data[TOFINO2_CTRL_DIGEST_SIZE]; /* Computed digest */
    uint32_t data_len;                      /* Data length */
    uint32_t key_len;                       /* Key length */
    uint8_t key[64];                        /* Digest key */
    uint64_t computed;                      /* Times computed */
    uint64_t verified;                      /* Times verified */
    uint64_t errors;                        /* Verification errors */
    uint8_t enabled;                        /* Digest enabled */
    uint8_t pad[3];
};

/* ============================================================================ */
/* Counter API                                                                    */
/* ============================================================================ */

int tofino2_ctrl_counter_allocate(uint32_t *counter_id);
int tofino2_ctrl_counter_free(uint32_t counter_id);
int tofino2_ctrl_counter_get(uint32_t counter_id,
                              struct tofino2_ctrl_counter *counter);
int tofino2_ctrl_counter_set(uint32_t counter_id,
                              const struct tofino2_ctrl_counter *counter);
int tofino2_ctrl_counter_read(uint32_t counter_id, uint64_t *value);
int tofino2_ctrl_counter_write(uint32_t counter_id, uint64_t value);
int tofino2_ctrl_counter_reset(uint32_t counter_id);
int tofino2_ctrl_counter_reset_all(void);
int tofino2_ctrl_counter_get_stats(uint32_t counter_id,
                                    uint64_t *hits, uint64_t *misses);
int tofino2_ctrl_counter_get_all(struct tofino2_ctrl_counter **counters,
                                  uint32_t *num_counters);
int tofino2_ctrl_counter_get_by_name(const char *name,
                                      struct tofino2_ctrl_counter *counter);
int tofino2_ctrl_counter_set_name(uint32_t counter_id, const char *name);
int tofino2_ctrl_counter_get_name(uint32_t counter_id, char *name,
                                   uint32_t name_size);

/* ============================================================================ */
/* Meter API                                                                      */
/* ============================================================================ */

int tofino2_ctrl_meter_allocate(uint32_t *meter_id,
                                 enum tofino2_ctrl_meter_mode mode);
int tofino2_ctrl_meter_free(uint32_t meter_id);
int tofino2_ctrl_meter_get(uint32_t meter_id,
                            struct tofino2_ctrl_meter *meter);
int tofino2_ctrl_meter_set(uint32_t meter_id,
                            const struct tofino2_ctrl_meter *meter);
int tofino2_ctrl_meter_configure(uint32_t meter_id,
                                  uint32_t cir, uint32_t pir,
                                  uint32_t cbs, uint32_t pbs);
int tofino2_ctrl_meter_check(uint32_t meter_id, uint32_t packet_len,
                              uint8_t *color);
int tofino2_ctrl_meter_get_stats(uint32_t meter_id,
                                  uint64_t *green_pkts,
                                  uint64_t *yellow_pkts,
                                  uint64_t *red_pkts);
int tofino2_ctrl_meter_get_all(struct tofino2_ctrl_meter **meters,
                                uint32_t *num_meters);
int tofino2_ctrl_meter_get_by_name(const char *name,
                                    struct tofino2_ctrl_meter *meter);
int tofino2_ctrl_meter_set_name(uint32_t meter_id, const char *name);
int tofino2_ctrl_meter_get_name(uint32_t meter_id, char *name,
                                 uint32_t name_size);

/* ============================================================================ */
/* DMA API                                                                        */
/* ============================================================================ */

int tofino2_ctrl_dma_allocate(uint32_t *channel_id,
                               enum tofino2_ctrl_dma_direction dir);
int tofino2_ctrl_dma_free(uint32_t channel_id);
int tofino2_ctrl_dma_get(uint32_t channel_id,
                          struct tofino2_ctrl_dma *dma);
int tofino2_ctrl_dma_set(uint32_t channel_id,
                          const struct tofino2_ctrl_dma *dma);
int tofino2_ctrl_dma_start(uint32_t channel_id,
                            uint64_t src, uint64_t dst, uint32_t len);
int tofino2_ctrl_dma_stop(uint32_t channel_id);
int tofino2_ctrl_dma_abort(uint32_t channel_id);
int tofino2_ctrl_dma_poll(uint32_t channel_id, int timeout_ms);
int tofino2_ctrl_dma_wait(uint32_t channel_id, int timeout_ms);
int tofino2_ctrl_dma_get_status(uint32_t channel_id,
                                 enum tofino2_ctrl_dma_status *status);
int tofino2_ctrl_dma_get_completed(uint32_t channel_id, uint32_t *completed);
int tofino2_ctrl_dma_get_all(struct tofino2_ctrl_dma **dma_channels,
                              uint32_t *num_channels);
int tofino2_ctrl_dma_buf_alloc(uint32_t size,
                                uint8_t **virt_addr,
                                dma_addr_t *dma_addr);
int tofino2_ctrl_dma_buf_free(uint8_t *virt_addr, dma_addr_t dma_addr,
                               uint32_t size);

/* ============================================================================ */
/* Digest API                                                                     */
/* ============================================================================ */

int tofino2_ctrl_digest_allocate(uint32_t *digest_id,
                                  enum tofino2_ctrl_digest_type type);
int tofino2_ctrl_digest_free(uint32_t digest_id);
int tofino2_ctrl_digest_get(uint32_t digest_id,
                             struct tofino2_ctrl_digest *digest);
int tofino2_ctrl_digest_set(uint32_t digest_id,
                             const struct tofino2_ctrl_digest *digest);
int tofino2_ctrl_digest_compute(uint32_t digest_id,
                                 const uint8_t *data, uint32_t len);
int tofino2_ctrl_digest_verify(uint32_t digest_id,
                                const uint8_t *data, uint32_t len);

#endif /* TOFINO2_CTRL_H */
