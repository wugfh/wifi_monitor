#pragma once

#include <netlink/netlink.h>
#include <nl80211.h>
#include <netlink/cache.h>
#include <netlink/genl/genl.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <net/ethernet.h>
#include <stdbool.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <endian.h>
#include <pthread.h>

#include "nl80211.h"
#include "ieee80211.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IW_COMMOND_BUFFER_LEN       100
#define NSS_MAX                     8

struct ifphy{
    uint8_t     retry_short;
    uint8_t     retry_long;
    uint8_t     bands;

    uint32_t    rts_threshold;
    uint32_t    frag_threshold;
};

struct ifstat{
    uint32_t        phy_id;
    uint32_t        ifidx;
    uint32_t        wdev;
    uint32_t        iftype;

    uint32_t        freq;
    uint32_t        ch_width;
    uint32_t        ch_type;
    uint32_t        freq_center1;
    uint32_t        freq_center2;

    double          tx_power;
    uint32_t        power_save;

    char            ssid[64];

    struct ifphy    phy_info;
};


struct rate_stat{
    uint32_t tx_rate;  // 100 kbps
    char mcs_info[20];
    enum{
        MCS_INFO_BUFFER_LEN = 20,
    }buffer_len;
};

/**
 * @overmem: number of drops due to memory limit overflow
 *      (only for per-phy stats)
 * @overlimit: number of drops due to queue space overflow
 * 
 * 
 * 
 * ***/
struct txq_stat{
    uint64_t backlog_pack;
    uint64_t tx_pack;
    uint64_t tx_packdrop;
    uint64_t overmem;
    uint64_t overlimit;
};

typedef void (*add_msg_t)(struct nl_msg*, void*);

struct nl_cmd{
    enum nl80211_commands   cmd;
    struct nl_sock*         sk;
    int                     flags;

    struct nl_msg*          msg;

    nl_recvmsg_msg_cb_t     handler;
    void*                   handler_args;

    add_msg_t               msg_handler;
    void*                   msg_handler_args;
};

struct sta_info{
    uint32_t            ifidx;

    uint64_t            rx_bytes;
    uint32_t            rx_packets;
    struct rate_stat    rx_rate;
    uint32_t            rx_drop_misc;

    uint64_t            tx_bytes;
    uint32_t            tx_packets;
    struct rate_stat    tx_rate;
    uint32_t            tx_failed;
    uint32_t            tx_retrys;
};

struct bss_info{
    struct ether_addr   bssid;
    int8_t              bss_signal;
    uint8_t             bss_signal_qual;
    uint32_t            status;
};

struct link_stat_buffer{
    struct sta_info*    l_stat;
    int                 link_cnt;
    pthread_t           mtid;
};

typedef enum{
    LEGACY,
    HT,
    VHT,
    HE,
}en_mcs_family;

typedef enum{
    _2_4G,
    _5G,
    _6G,
}en_mcs_freq;

struct mcs_status{
    en_mcs_family               family;
    en_mcs_freq                 freq;
    uint8_t                     mcs_index;
    uint16_t                    legacy_rate;
    uint16_t                    nss;
};

/**
 * @brief interface type enum, do not use AP
 * 
 * **/
typedef enum{
    MANAGED,
    MONITOR,
    AP,
}interface_type;

struct interface_status{
    uint8_t         have_mac;
    uint8_t         mac_addr[ETH_ALEN];
    interface_type  type;
    uint8_t         name[20];
};

typedef struct bss_info             bss_info_t;
typedef struct nl_cmd               nl_cmd_t;
typedef struct sta_info             sta_info_t;
typedef struct ifstat               ifstat_t;
typedef struct ifphy                ifphy_t;
typedef struct freq_stat            freq_stat_t;
typedef struct rate_stat            rate_stat_t;
typedef struct mcs_status           mcs_status_t;
typedef struct interface_status     interface_status_t;

int register_dev(struct link_stat_buffer* lstat_buffer, char* dev);
int link_monitor_init(struct link_stat_buffer* lstat_buffer);
void nl80211_set_mcs(mcs_status_t* mcs, char* dev);
void nl80211_add_interface(interface_status_t* status, char* dev);
void nl80211_get_mac(uint8_t* mac_buff, char* dev);
void nl80211_diable_ampdu(uint16_t tid_bismask, char* dev);
void nl80211_del_interface(char* dev);

#ifdef __cplusplus
}
#endif