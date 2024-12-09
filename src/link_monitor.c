#include <stdio.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netlink/genl/ctrl.h>
#include <errno.h>
#include "link_monitor.h"
#include "util.h"

static char iw_cmbuffer[IW_COMMOND_BUFFER_LEN];

/* Predefined handlers, stolen from iw:iw.c */
static inline int error_handler(struct sockaddr_nl __attribute__((unused))*nla,
				struct nlmsgerr *err, void *arg)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)err - 1;
	int len = nlh->nlmsg_len;
	struct nlattr *attrs;
	struct nlattr *tb[NLMSGERR_ATTR_MAX + 1];
	int *ret = arg;
	int ack_len = sizeof(*nlh) + sizeof(int) + sizeof(*nlh);

	if (err->error > 0) {
		/*
		 * This is illegal, per netlink(7), but not impossible (think
		 * "vendor commands"). Callers really expect negative error
		 * codes, so make that happen.
		 */
		fprintf(stderr,
			"ERROR: received positive netlink error code %d\n",
			err->error);
		*ret = -EPROTO;
	} else {
		*ret = err->error;
	}

	if (!(nlh->nlmsg_flags & NLM_F_ACK_TLVS))
		return NL_STOP;

	if (!(nlh->nlmsg_flags & NLM_F_CAPPED))
		ack_len += err->msg.nlmsg_len - sizeof(*nlh);

	if (len <= ack_len)
		return NL_STOP;

	attrs = (void *)((unsigned char *)nlh + ack_len);
	len -= ack_len;

	nla_parse(tb, NLMSGERR_ATTR_MAX, attrs, len, NULL);
	if (tb[NLMSGERR_ATTR_MSG]) {
		len = strnlen((char *)nla_data(tb[NLMSGERR_ATTR_MSG]),
			      nla_len(tb[NLMSGERR_ATTR_MSG]));
		fprintf(stderr, "kernel reports: %*s\n", len,
			(char *)nla_data(tb[NLMSGERR_ATTR_MSG]));
	}

	return NL_STOP;
}

static inline int finish_handler(struct nl_msg __attribute__((unused))*msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static inline int ack_handler(struct nl_msg __attribute__((unused))*msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

static inline int no_seq_check(struct nl_msg __attribute__((unused))*msg,
			       void __attribute__((unused))*arg)
{
	return NL_OK;
}

int valid_handler(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}


/**
 * @brief Sends netlink messages and handles feedback from the kernel
 * 
 * @param cmd the nl80211 command 
 * 
 * **/
static int handle_interface_cmd(nl_cmd_t* cmd, uint32_t ifindex){
    struct nl_cb* cb;
    struct nl_msg* msg;

    if(cmd->sk == NULL){
        cmd->sk = nl_socket_alloc();
    }
    if(genl_connect(cmd->sk)){
        perror("can not connect to genetlink");
    }

    static int nl80211_id = -1;
    if(nl80211_id < 0)
        nl80211_id = genl_ctrl_resolve(cmd->sk, "nl80211");

    msg = nlmsg_alloc();
    cb = nl_cb_alloc(NL_CB_DEFAULT);

    //add msg header
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, cmd->flags, cmd->cmd, 0);

    nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);

    if(cmd->msg_handler != NULL){
        cmd->msg_handler(msg, cmd->msg_handler_args);
    }

    int ret = nl_send_auto(cmd->sk, msg);

    if(ret < 0){
        perror("sending netlink message failed");
    }

    if(ret < 0){
        perror("nl send msg fail");
        return -1;
    }
    
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &ret);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);

    if(cmd->handler != NULL){
        nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, cmd->handler, cmd->handler_args);
    }
    else{
        nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);
    }

    while(ret > 0){
        nl_recvmsgs(cmd->sk, cb);
    }
    nl_cb_put(cb);
    nlmsg_free(msg);
    nl_socket_free(cmd->sk);
    return ret;
}

static int parse_bitrate(struct nlattr* bitrate_attr, void* args){
    int rate = 0;
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

    rate_stat_t* r_stat = (rate_stat_t*)args;

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
			     bitrate_attr, rate_policy)) {
		perror("failed to parse nested rate attributes!"); 
		return NL_SKIP;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32])
		rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	else if (rinfo[NL80211_RATE_INFO_BITRATE])
		rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	if (rate > 0){
        r_stat->tx_rate = rate;
    }
	else{
        r_stat->tx_rate = 0; 
    }

	if (rinfo[NL80211_RATE_INFO_MCS]){
		snprintf(r_stat->mcs_info, MCS_INFO_BUFFER_LEN,
				" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
    }
	if (rinfo[NL80211_RATE_INFO_VHT_MCS])
		snprintf(r_stat->mcs_info, MCS_INFO_BUFFER_LEN,
				" VHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
	if (rinfo[NL80211_RATE_INFO_HE_MCS])
		snprintf(r_stat->mcs_info, MCS_INFO_BUFFER_LEN,
				" HE-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_HE_MCS]));
	if (rinfo[NL80211_RATE_INFO_EHT_MCS])
		snprintf(r_stat->mcs_info, MCS_INFO_BUFFER_LEN,
				" EHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_EHT_MCS]));
	// if (rinfo[NL80211_RATE_INFO_EHT_NSS])
	// 	snprintf(r_stat->mcs_info, r_stat->mcs_info[MCS_INFO_BUFFER_LEN],
	// 			" EHT-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_EHT_NSS]));
	// if (rinfo[NL80211_RATE_INFO_EHT_GI])
	// 	snprintf(r_stat->mcs_info, r_stat->mcs_info[MCS_INFO_BUFFER_LEN],
	// 			" EHT-GI %d", nla_get_u8(rinfo[NL80211_RATE_INFO_EHT_GI]));
	// if (rinfo[NL80211_RATE_INFO_EHT_RU_ALLOC])
	// 	snprintf(r_stat->mcs_info, r_stat->mcs_info[MCS_INFO_BUFFER_LEN],
	// 			" EHT-RU-ALLOC %d", nla_get_u8(rinfo[NL80211_RATE_INFO_EHT_RU_ALLOC]));
    return NL_SKIP;
}

static int parse_bss(struct nlattr* bss_attr, void* args){
    bss_info_t* l_stat = (bss_info_t*)args;
    struct nlattr* bss[NL80211_BSS_MAX+1];
    static struct nla_policy bss_policy[NL80211_BSS_MAX+1] = {
        [NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
        [NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
        [NL80211_BSS_SIGNAL_MBM] = { .type = NLA_S32 },
        [NL80211_BSS_CHAN_WIDTH] = { .type = NLA_U32 },
        [NL80211_BSS_TSF] = { .type = NLA_U64 },
        [NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16},
        [NL80211_BSS_STATUS] = { .type = NLA_U32 },
        [NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 }
    };

    if(nla_parse_nested(bss, NL80211_BSS_MAX, bss_attr, bss_policy) < 0){
        return NL_SKIP;
    }

    if(bss[NL80211_BSS_SIGNAL_UNSPEC])
        l_stat->bss_signal_qual = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);
    
    if(bss[NL80211_BSS_SIGNAL_MBM]){
        l_stat->bss_signal = nla_get_s32(bss[NL80211_BSS_SIGNAL_MBM]);
        l_stat->bss_signal /= 100;
    }

    l_stat->status = nla_get_u32(bss[NL80211_BSS_STATUS]);
    memcpy(&l_stat->bssid, nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);
    return NL_SKIP;
}

static int parse_tid_stats(struct nlattr* sta_attr_info, void* args){

}

static int parse_sta(struct nlattr* sta_attr_info, void* args){
    struct nlattr* sta_info[NL80211_STA_INFO_MAX+1];
    struct nlattr* sta_bss[NL80211_STA_BSS_PARAM_MAX+1];
    struct nl80211_sta_flag_update* sta_flags;
    sta_info_t* sta = (sta_info_t*)args;
    static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_DROP_MISC] = { .type = NLA_U64}
	};

    nla_parse_nested(sta_info, NL80211_STA_INFO_MAX, sta_attr_info, stats_policy);

    if(sta_info[NL80211_STA_INFO_TX_BITRATE]) {
		parse_bitrate(sta_info[NL80211_STA_INFO_TX_BITRATE], &sta->tx_rate);
	}

    if(sta_info[NL80211_STA_INFO_RX_BITRATE]){
        parse_bitrate(sta_info[NL80211_STA_INFO_RX_BITRATE], &sta->rx_rate);
    }

    if(sta_info[NL80211_STA_INFO_RX_BYTES64]){
        sta->rx_bytes = nla_get_u64(sta_info[NL80211_STA_INFO_RX_BYTES64]);
    }

    if(sta_info[NL80211_STA_INFO_TX_BYTES64]){
        sta->tx_bytes = nla_get_u64(sta_info[NL80211_STA_INFO_TX_BYTES64]);
    }

    if(sta_info[NL80211_STA_INFO_TX_PACKETS]){
        sta->tx_packets = nla_get_u32(sta_info[NL80211_STA_INFO_TX_PACKETS]);
    }

    if(sta_info[NL80211_STA_INFO_RX_PACKETS]){
        sta->rx_packets = nla_get_u32(sta_info[NL80211_STA_INFO_RX_PACKETS]);
    }

    if(sta_info[NL80211_STA_INFO_TX_RETRIES]){
        sta->tx_retrys = nla_get_u32(sta_info[NL80211_STA_INFO_TX_RETRIES]);
    }

    if(sta_info[NL80211_STA_INFO_TX_FAILED]){
        sta->tx_failed = nla_get_u32(sta_info[NL80211_STA_INFO_TX_FAILED]);
    }

    if(sta_info[NL80211_STA_INFO_RX_DROP_MISC]){
        sta->rx_drop_misc = nla_get_u64(sta_info[NL80211_STA_INFO_RX_DROP_MISC]);
    }

    return NL_SKIP;
}

static int bss_handler(struct nl_msg* msg, void* args){
    bss_info_t* l_stat = (bss_info_t*)args;
    struct nlattr* tb[NL80211_ATTR_MAX+1];
    struct genlmsghdr* gnmhdr = nlmsg_data(nlmsg_hdr(msg));

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnmhdr, 0), genlmsg_attrlen(gnmhdr, 0), NULL);

    if(!tb[NL80211_ATTR_BSS]){
        return NL_SKIP;
    }

    return parse_bss(tb[NL80211_ATTR_BSS], args);
}

static int sta_hander(struct nl_msg* msg, void* args){
    sta_info_t* l_stat = (sta_info_t*)args;
    struct nlattr* tb[NL80211_ATTR_MAX];
    struct genlmsghdr* gnmhdr = nlmsg_data(nlmsg_hdr(msg));

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnmhdr, 0), genlmsg_attrlen(gnmhdr, 0), NULL);

    if(!tb[NL80211_ATTR_STA_INFO]){
        return NL_SKIP;
    }
    return parse_sta(tb[NL80211_ATTR_STA_INFO], args);
}

static int rate_get_hander(struct nl_msg* msg, void* args){
    struct nlattr* tb[NL80211_ATTR_MAX+1];
    struct genlmsghdr* gnmhdr = nlmsg_data(nlmsg_hdr(msg));

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnmhdr, 0), genlmsg_attrlen(gnmhdr, 0), NULL);

    if(!tb[NL80211_ATTR_STA_INFO]){
        return NL_SKIP;
    }
    return parse_sta(tb[NL80211_ATTR_STA_INFO], args);
}


static int nl80211_get_sta_info(sta_info_t* sta, int ifindex){
    nl_cmd_t cmd_sta = {
        .cmd = NL80211_CMD_GET_STATION,
        .flags = NLM_F_DUMP,
        .handler = sta_hander,
    };
    cmd_sta.handler_args = sta;

    handle_interface_cmd(&cmd_sta, ifindex);
}


void* link_monitor_thread_entry(void* args){
    printf("begin to monitor link\n");
    struct link_stat_buffer* lstat_buffer = (struct link_stat_buffer*)args;
    while(lstat_buffer->l_stat == NULL){
        sleep(0);
    }
    while(1){
        for(int i = 0; i < lstat_buffer->link_cnt; ++i){
            nl80211_get_sta_info(&lstat_buffer->l_stat[i], lstat_buffer->l_stat[i].ifidx);
        }
        sleep(1);
    }
}   

int link_monitor_init(struct link_stat_buffer* lstat_buffer){
    lstat_buffer->l_stat = NULL;
    lstat_buffer->link_cnt = 0;
    int status = pthread_create(&lstat_buffer->mtid, NULL, link_monitor_thread_entry, lstat_buffer);
    if(status != 0){
        perror("monitor thread create fail");
        return -1;
    }
    return 0;
}

int register_dev(struct link_stat_buffer* lstat_buffer, char* dev){
    if(lstat_buffer->l_stat == NULL){
        lstat_buffer->l_stat = malloc((lstat_buffer->link_cnt+1)*sizeof(sta_info_t));
    }
    else {
        lstat_buffer->l_stat = realloc(lstat_buffer->l_stat, (lstat_buffer->link_cnt+1)*sizeof(sta_info_t));
    }
    lstat_buffer->l_stat[lstat_buffer->link_cnt].ifidx = if_nametoindex(dev);
    lstat_buffer->link_cnt = lstat_buffer->link_cnt+1;
    
    return 0;
}


static void link_set_mcs_attach_msg(struct nl_msg* msg, void* args){
    mcs_status_t* mcs = (void*)args;
    int attr = NL80211_ATTR_TX_RATES;
    struct nlattr* nl_rate = nla_nest_start(msg, attr);
    struct nlattr *nl_band;
    switch (mcs->freq){
    case _2_4G:
        nl_band = nla_nest_start(msg, NL80211_BAND_2GHZ);
        break;
    case _5G:
        nl_band = nla_nest_start(msg, NL80211_BAND_5GHZ);
        break;
    case _6G:
        nl_band = nla_nest_start(msg, NL80211_BAND_6GHZ);
        break;
    default:
        break;
    }

    switch (mcs->family){
    case LEGACY:
        nla_put(msg, NL80211_TXRATE_LEGACY, 2, &mcs->legacy_rate);
        break;
    case HT:
        nla_put(msg, NL80211_TXRATE_HT, 1, &mcs->mcs_index);
        break;
    case VHT:
        struct nl80211_txrate_vht vht_mcs = {};
        vht_mcs.mcs[mcs->nss-1] |= (1<<mcs->mcs_index);
        nla_put(msg, NL80211_TXRATE_VHT, sizeof(vht_mcs), &vht_mcs);
        break;
    case HE:
        struct nl80211_txrate_he he_mcs = {};
        he_mcs.mcs[mcs->nss-1] |= (1<<mcs->mcs_index);
        nla_put(msg, NL80211_TXRATE_HE, sizeof(he_mcs), &he_mcs);
        break;
    default:
        break;
    }
    nla_nest_end(msg, nl_band);
    nla_nest_end(msg, nl_rate);
}


void nl80211_set_mcs(mcs_status_t* mcs, char* dev){
    nl_cmd_t cmd_set_rate = {
        .cmd = NL80211_CMD_SET_TX_BITRATE_MASK,
        .flags = 0,
        .msg_handler = link_set_mcs_attach_msg,
        .msg_handler_args = mcs,
    };

    int ifindex = if_nametoindex(dev);
    handle_interface_cmd(&cmd_set_rate, ifindex);
}   

static void interface_attach_msg(struct nl_msg* msg, void *args){
    interface_status_t* if_status = (interface_status_t*)args;
    nla_put(msg, NL80211_ATTR_IFNAME, (int) strlen(if_status->name) + 1, if_status->name);

    uint32_t type_tmp = NL80211_IFTYPE_UNSPECIFIED;
    switch (if_status->type){
    case MANAGED:
        type_tmp = NL80211_IFTYPE_STATION;
        break;
    case MONITOR:
        type_tmp = NL80211_IFTYPE_MONITOR;
        break;
    case AP:
        // have no effect, it will create a interface typed managed
        type_tmp = NL80211_IFTYPE_AP;
        break;
    default:
        break;
    }
    nla_put_u32(msg, NL80211_ATTR_IFTYPE, type_tmp);
    if(if_status->have_mac)
	    nla_put(msg, NL80211_ATTR_MAC, ETH_ALEN, if_status->mac_addr);
} 

void nl80211_add_interface(interface_status_t* status, char* dev){
    nl_cmd_t cmd_set_rate = {
        .cmd = NL80211_CMD_NEW_INTERFACE,
        .flags = 0,
        .msg_handler = interface_attach_msg,
        .msg_handler_args = status,
    };

    int ifindex = if_nametoindex(dev);
    handle_interface_cmd(&cmd_set_rate, ifindex);

    enable_interface(status->name);
}

/****
 * @brief not be tested
 * 
 * 
*/
void nl80211_del_interface(char* dev){
    nl_cmd_t cmd_set_rate = {
        .cmd = NL80211_CMD_DEL_INTERFACE,
        .flags = 0,
        .msg_handler = NULL,
        .msg_handler_args = NULL,
    };

    int ifindex = if_nametoindex(dev);
    handle_interface_cmd(&cmd_set_rate, ifindex);
}


static int mac_hander(struct nl_msg* msg, void* args){
    struct nlattr* tb[NL80211_ATTR_MAX];
    struct genlmsghdr* gnmhdr = nlmsg_data(nlmsg_hdr(msg));

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnmhdr, 0), genlmsg_attrlen(gnmhdr, 0), NULL);

    if(!tb[NL80211_ATTR_MAC]){
        return NL_SKIP;
    }
    memcpy(args, nla_data(tb[NL80211_ATTR_MAC]), ETH_ALEN);
    return NL_SKIP;
}


void nl80211_get_mac(uint8_t* mac_buff, char* dev){
    nl_cmd_t cmd_mac = {
        .cmd = NL80211_CMD_GET_INTERFACE,
        .flags = NLM_F_DUMP,
        .handler = mac_hander,
    };
    cmd_mac.handler_args = mac_buff;
    int ifindex = if_nametoindex(dev);
    handle_interface_cmd(&cmd_mac, ifindex);
}

/**
 * @brief have no effect
 * 
 * 
***/
static void ampdu_disable_attach_msg(struct nl_msg* msg, void *args){
    struct nlattr* tid_config;
    uint16_t mask = *(uint16_t*)args;
    tid_config = nla_nest_start(msg, NL80211_ATTR_TID_CONFIG);
    nla_put_u16(msg, NL80211_TID_CONFIG_ATTR_TIDS, mask);
    nla_put_u8(msg, NL80211_TID_CONFIG_ATTR_AMPDU_CTRL, NL80211_TID_CONFIG_DISABLE);
    nla_nest_end(msg, tid_config);
}

/**
 * @brief have no effect
 * 
 * 
***/
void nl80211_diable_ampdu(uint16_t tid_bismask, char* dev){
    nl_cmd_t cmd_ampdu = {
        .cmd = NL80211_CMD_SET_TID_CONFIG,
        .flags = 0,
        .msg_handler = ampdu_disable_attach_msg,
        .msg_handler_args = &tid_bismask,
    };
    int ifindex = if_nametoindex(dev);
    handle_interface_cmd(&cmd_ampdu, ifindex);
}