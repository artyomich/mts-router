/*
 * mbs_dpdk.h — DPDK HAL for MTS-MB-3000
 *
 * MTS Mobile Backhaul — DPDK PMD HAL
 */

#ifndef MBS_DPDK_H
#define MBS_DPDK_H

#include "mbs_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DPDK port configuration */
#define MBS_DPDK_DEFAULT_MBUFS 8192
#define MBS_DPDK_DEFAULT_MBUF_SIZE 2048
#define MBS_DPDK_DEFAULT_RX_QUEUES 4
#define MBS_DPDK_DEFAULT_TX_QUEUES 4
#define MBS_DPDK_DEFAULT_RX_DESC 512
#define MBS_DPDK_DEFAULT_TX_DESC 512

/* DPDK PMD types */
#define MBS_DPDK_PMD_NETMAP "netmap"
#define MBS_DPDK_PMD_VHOST "vhost"
#define MBS_DPDK_PMD_DPAA2 "dpaa2"
#define MBS_DPDK_PMD_AF_XDP "af_xdp"

/* RSS hash types */
#define MBS_DPDK_RSS_HASH_UDP "udp"
#define MBS_DPDK_RSS_HASH_TCP "tcp"
#define MBS_DPDK_RSS_HASH_IP "ip"

/* ==================== DPDK Initialization ==================== */

/* Initialize DPDK */
int mbs_dpdk_init(const char *socket_id,
                   uint32_t num_mbufs,
                   uint32_t mbuf_size);

/* Shutdown DPDK */
int mbs_dpdk_exit(void);

/* ==================== Port Configuration ==================== */

/* Configure DPDK port */
int mbs_dpdk_port_configure(uint32_t port_id,
                             uint32_t num_rx_queues,
                             uint32_t num_tx_queues,
                             uint32_t rx_desc_len,
                             uint32_t tx_desc_len,
                             bool enable_rss);

/* Get DPDK port configuration */
int mbs_dpdk_port_get_config(uint32_t port_id,
                              uint32_t *num_rx_queues,
                              uint32_t *num_tx_queues,
                              uint32_t *rx_desc_len,
                              uint32_t *tx_desc_len);

/* ==================== Packet Processing ==================== */

/* Receive packets from port */
int mbs_dpdk_port_recv(uint32_t port_id,
                        void **packets,
                        uint32_t max_packets,
                        uint32_t *num_recv);

/* Transmit packets to port */
int mbs_dpdk_port_send(uint32_t port_id,
                        void **packets,
                        uint32_t num_packets);

/* ==================== Statistics ==================== */

/* Get DPDK port statistics */
int mbs_dpdk_port_get_stats(uint32_t port_id,
                             mbs_dpdk_stats_t *stats);

/* Get all DPDK port statistics */
int mbs_dpdk_port_get_all_stats(mbs_dpdk_stats_t *stats,
                                 uint32_t *count);

/* Reset DPDK port statistics */
int mbs_dpdk_port_reset_stats(uint32_t port_id);

/* ==================== RSS Configuration ==================== */

/* Configure RSS hash */
int mbs_dpdk_rss_configure(uint32_t port_id,
                            const char *hash_types,
                            const uint8_t *key,
                            uint32_t key_len);

/* Get RSS configuration */
int mbs_dpdk_rss_get_config(uint32_t port_id,
                             char *hash_types,
                             uint32_t hash_types_len,
                             uint8_t *key,
                             uint32_t key_len);

/* ==================== Queue Operations ==================== */

/* Create RX queue */
int mbs_dpdk_rx_queue_create(uint32_t port_id,
                              uint32_t queue_id,
                              uint32_t num_desc);

/* Destroy RX queue */
int mbs_dpdk_rx_queue_destroy(uint32_t port_id,
                               uint32_t queue_id);

/* Create TX queue */
int mbs_dpdk_tx_queue_create(uint32_t port_id,
                              uint32_t queue_id,
                              uint32_t num_desc);

/* Destroy TX queue */
int mbs_dpdk_tx_queue_destroy(uint32_t port_id,
                               uint32_t queue_id);

/* ==================== Performance Monitoring ==================== */

/* Get DPDK performance counters */
int mbs_dpdk_get_perf_counters(double *throughput_mbps,
                                double *cpu_usage,
                                uint64_t *packets_processed,
                                uint64_t *packets_dropped);

/* Start performance monitoring */
int mbs_dpdk_perf_start(void);

/* Stop performance monitoring */
int mbs_dpdk_perf_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MBS_DPDK_H */
