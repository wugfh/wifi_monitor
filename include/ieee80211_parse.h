#pragma once

/****
 * ieee header define 
 * stolen from tcpdump
 * 
 * **/

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/if_ether.h>
#include <stdint.h>
#include "ieee80211.h"

namespace ieee80211_parser{
	

/* Lengths of 802.11 header components. */
#define	IEEE802_11_FC_LEN		    2
#define	IEEE802_11_DUR_LEN		    2
#define	IEEE802_11_DA_LEN		    6
#define	IEEE802_11_SA_LEN		    6
#define	IEEE802_11_BSSID_LEN		6
#define	IEEE802_11_RA_LEN		    6
#define	IEEE802_11_TA_LEN		    6
#define	IEEE802_11_ADDR1_LEN		6
#define	IEEE802_11_SEQ_LEN		    2
#define	IEEE802_11_CTL_LEN		    2
#define	IEEE802_11_CARRIED_FC_LEN	2
#define	IEEE802_11_HT_CONTROL_LEN	4
#define	IEEE802_11_IV_LEN		    3
#define	IEEE802_11_KID_LEN		    1

/* Frame check sequence length. */
#define	IEEE802_11_FCS_LEN		    4

/* Lengths of beacon components. */
#define	IEEE802_11_TSTAMP_LEN		8
#define	IEEE802_11_BCNINT_LEN		2
#define	IEEE802_11_CAPINFO_LEN		2
#define	IEEE802_11_LISTENINT_LEN	2

#define	IEEE802_11_AID_LEN		    2
#define	IEEE802_11_STATUS_LEN		2
#define	IEEE802_11_REASON_LEN		2

/* Length of previous AP in reassociation frame */
#define	IEEE802_11_AP_LEN		    6

#define	T_MGMT 0x0  /* management */
#define	T_CTRL 0x1  /* control */
#define	T_DATA 0x2 /* data */
#define	T_RESV 0x3  /* reserved */

#define	ST_ASSOC_REQUEST	0x0
#define	ST_ASSOC_RESPONSE	0x1
#define	ST_REASSOC_REQUEST	0x2
#define	ST_REASSOC_RESPONSE	0x3
#define	ST_PROBE_REQUEST	0x4
#define	ST_PROBE_RESPONSE	0x5
/* RESERVED			0x6  */
/* RESERVED			0x7  */
#define	ST_BEACON		0x8
#define	ST_ATIM			0x9
#define	ST_DISASSOC		0xA
#define	ST_AUTH			0xB
#define	ST_DEAUTH		0xC
#define	ST_ACTION		0xD
/* RESERVED			0xE  */
/* RESERVED			0xF  */

#define CTRL_CONTROL_WRAPPER	0x7
#define	CTRL_BAR	            0x8
#define	CTRL_BA		            0x9
#define	CTRL_PS_POLL	        0xA
#define	CTRL_RTS	            0xB
#define	CTRL_CTS	            0xC
#define	CTRL_ACK	            0xD
#define	CTRL_CF_END	            0xE
#define	CTRL_END_ACK	        0xF

#define	DATA_DATA			        0x0
#define	DATA_DATA_CF_ACK		    0x1
#define	DATA_DATA_CF_POLL		    0x2
#define	DATA_DATA_CF_ACK_POLL       0x3
#define	DATA_NODATA			        0x4
#define	DATA_NODATA_CF_ACK		    0x5
#define	DATA_NODATA_CF_POLL		    0x6
#define	DATA_NODATA_CF_ACK_POLL		0x7

#define DATA_QOS_DATA			    0x8
#define DATA_QOS_DATA_CF_ACK		0x9
#define DATA_QOS_DATA_CF_POLL		0xA
#define DATA_QOS_DATA_CF_ACK_POLL	0xB
#define DATA_QOS_NODATA			    0xC
#define DATA_QOS_CF_POLL_NODATA		0xE
#define DATA_QOS_CF_ACK_POLL_NODATA	0xF

/*
 * The subtype field of a data frame is, in effect, composed of 4 flag
 * bits - CF-Ack, CF-Poll, Null (means the frame doesn't actually have
 * any data), and QoS.
 */
#define DATA_FRAME_IS_CF_ACK(x)		((x) & 0x01)
#define DATA_FRAME_IS_CF_POLL(x)	((x) & 0x02)
#define DATA_FRAME_IS_NULL(x)		((x) & 0x04)
#define DATA_FRAME_IS_QOS(x)		((x) & 0x08)

/*
 * Bits in the frame control field.
 */
#define	FC_VERSION(fc)		((fc) & 0x3)
#define	FC_TYPE(fc)		    (((fc) >> 2) & 0x3)
#define	FC_SUBTYPE(fc)		(((fc) >> 4) & 0xF)
#define	FC_TO_DS(fc)		((fc) & 0x0100)
#define	FC_FROM_DS(fc)		((fc) & 0x0200)
#define	FC_MORE_FLAG(fc)	((fc) & 0x0400)
#define	FC_RETRY(fc)		((fc) & 0x0800)
#define	FC_POWER_MGMT(fc)	((fc) & 0x1000)
#define	FC_MORE_DATA(fc)	((fc) & 0x2000)
#define	FC_PROTECTED(fc)	((fc) & 0x4000)
#define	FC_ORDER(fc)		((fc) & 0x8000)

#define SSC_SN(ssc)			(((ssc) >> 4) & 0xFFF)
#define SSC_FG(ssc)			((ssc) & 0xF)

struct mgmt_header_t {
	uint16_t	frame_ctl;
	uint16_t	duration;
	uint8_t	    da[ETH_ALEN];
	uint8_t	    sa[ETH_ALEN];
	uint8_t	    bssid[ETH_ALEN];
	uint16_t	seq_ctrl;
};

#define	MGMT_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
			 IEEE802_11_DA_LEN+IEEE802_11_SA_LEN+\
			 IEEE802_11_BSSID_LEN+IEEE802_11_SEQ_LEN)

#define	CAPABILITY_ESS(cap)	((cap) & 0x0001)
#define	CAPABILITY_IBSS(cap)	((cap) & 0x0002)
#define	CAPABILITY_CFP(cap)	((cap) & 0x0004)
#define	CAPABILITY_CFP_REQ(cap)	((cap) & 0x0008)
#define	CAPABILITY_PRIVACY(cap)	((cap) & 0x0010)

struct ssid_t {
	uint32_t		length;
	uint8_t		    ssid[33];  /* 32 + 1 for null */
};

struct rates_t {
	uint32_t	length;
	uint8_t		rate[16];
};

struct challenge_t {
	uint32_t	length;
	uint8_t		text[254]; /* 1-253 + 1 for null */
};

struct fh_t {
	uint32_t	length;
	uint16_t	dwell_time;
	uint8_t		hop_set;
	uint8_t	    hop_pattern;
	uint8_t		hop_index;
};

struct ds_t {
	uint32_t	length;
	uint8_t		channel;
};

struct cf_t {
	uint32_t	length;
	uint8_t		count;
	uint8_t		period;
	uint16_t	max_duration;
	uint16_t	dur_remaining;
};

struct tim_t {
	uint32_t	length;
	uint8_t		count;
	uint8_t		period;
	uint8_t		bitmap_control;
	uint8_t		bitmap[251];
};

struct meshid_t {
	uint32_t	length;
	uint8_t		meshid[33];  /* 32 + 1 for null */
};

#define	E_SSID		0
#define	E_RATES	    1
#define	E_FH		2
#define	E_DS		3
#define	E_CF		4
#define	E_TIM		5
#define	E_IBSS		6
#define	E_CHALLENGE	16
#define E_MESHID	114

struct mgmt_body_t {
	uint8_t		        timestamp[IEEE802_11_TSTAMP_LEN];
	uint16_t	        beacon_interval;
	uint16_t	        listen_interval;
	uint16_t	        status_code;
	uint16_t	        aid;
	uint16_t	        ap[IEEE802_11_AP_LEN];
	uint16_t	        reason_code;
	uint16_t	        auth_alg;
	uint16_t	        auth_trans_seq_num;
	int		            challenge_present;
	struct challenge_t  challenge;
	uint16_t	        capability_info;
	int		            ssid_present;
	struct ssid_t	    ssid;
	int		            rates_present;
	struct rates_t	    rates;
	int		            ds_present;
	struct ds_t	        ds;
	int		            cf_present;
	struct cf_t	        cf;
	int		            fh_present;
	struct fh_t	        fh;
	int		            tim_present;
	struct tim_t	    tim;
	int		            meshid_present;
	struct meshid_t	    meshid;
	uint8_t 			category;
    uint8_t 			action;
};

/**
 * @brief Control Wrapper: Encapsulates another control frame for additional processing.
 * 
 * **/
struct ctrl_control_wrapper_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    addr1[ETH_ALEN];
	uint16_t	carried_fc[IEEE802_11_CARRIED_FC_LEN];
	uint16_t	ht_control[IEEE802_11_HT_CONTROL_LEN];
};

#define	CTRL_CONTROL_WRAPPER_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
					 IEEE802_11_ADDR1_LEN+\
					 IEEE802_11_CARRIED_FC_LEN+\
					 IEEE802_11_HT_CONTROL_LEN)

typedef enum{
	RTS_HDR,
	CTS_HDR,
	ACK_HDR,
	PS_POLL_HDR,
	CF_END_HDR,
	CF_END_ACK_HDR,
	BA_HDR,
	BAR_HDR,
}en_ctrl_hdr;

/***
 * @brief RTS (Request to Send): Indicates a device’s intention to send data to avoid collisions.
 * 
*/
struct ctrl_rts_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
	uint8_t	    ta[ETH_ALEN];
};

#define	CTRL_RTS_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
			 IEEE802_11_RA_LEN+IEEE802_11_TA_LEN)

/**
 * @brief CTS (Clear to Send): Sent in response to RTS, granting permission to send data.
 * 
 * **/
struct ctrl_cts_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
};

#define	CTRL_CTS_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+IEEE802_11_RA_LEN)

/**
 * @brief ACK (Acknowledgement): Confirms successful receipt of a data frame.
 * 
 * **/
struct ctrl_ack_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
	uint16_t 	ssc;
};

#define	CTRL_ACK_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+IEEE802_11_RA_LEN)

/***
 * @brief PS-Poll: Used by devices in power-saving mode to request data from the access point.
 * 
 * **/
struct ctrl_ps_poll_hdr_t {
	uint16_t	fc;
	uint16_t	aid;
	uint8_t	    bssid[ETH_ALEN];
	uint8_t	    ta[ETH_ALEN];
};

#define	CTRL_PS_POLL_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_AID_LEN+\
				 IEEE802_11_BSSID_LEN+IEEE802_11_TA_LEN)

/**
 * @brief CF-End: Marks the end of the contention-free period.
 * 
 * **/
struct ctrl_end_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
	uint8_t	    bssid[ETH_ALEN];
};

#define	CTRL_END_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
			 IEEE802_11_RA_LEN+IEEE802_11_BSSID_LEN)

/**
 * @brief CF-End + CF-ACK: Acknowledges the end of the contention-free period and the receipt of data.
 * 
 * **/
struct ctrl_end_ack_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
	uint8_t	    bssid[ETH_ALEN];
};

#define	CTRL_END_ACK_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
				 IEEE802_11_RA_LEN+IEEE802_11_BSSID_LEN)

/**
 * @brief Block Ack (BA): Acknowledges the receipt of a block of data frames.
 * 
 * **/
struct ctrl_ba_hdr_t {
	uint16_t	fc;
	uint16_t	duration;
	uint8_t	    ra[ETH_ALEN];
	uint8_t	    ta[ETH_ALEN];
	uint16_t 	ba_ctrl;
	uint16_t	ssc;
	uint64_t 	bitmap;
};

#define	CTRL_BA_HDRLEN	(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
			 IEEE802_11_RA_LEN+IEEE802_11_TA_LEN)

/**
 * @brief Block Ack Request (BAR): Requests an acknowledgment for a block of data frames.
 * 
 * **/
struct ctrl_bar_hdr_t {
	uint16_t	fc;
	uint16_t	dur;
	uint8_t	    ra[ETH_ALEN];
	uint8_t	    ta[ETH_ALEN];
	uint16_t	ctl;
	uint16_t	seq;
};

#define	CTRL_BAR_HDRLEN		(IEEE802_11_FC_LEN+IEEE802_11_DUR_LEN+\
				 IEEE802_11_RA_LEN+IEEE802_11_TA_LEN+\
				 IEEE802_11_CTL_LEN+IEEE802_11_SEQ_LEN)

struct meshcntl_t {
	uint8_t	    flags;
	uint8_t	    ttl;
	uint32_t	seq;
	uint8_t	    addr4[ETH_ALEN];
	uint8_t	    addr5[ETH_ALEN];
	uint8_t	    addr6[ETH_ALEN];
};

#define	IV_IV(iv)	((iv) & 0xFFFFFF)
#define	IV_PAD(iv)	(((iv) >> 24) & 0x3F)
#define	IV_KEYID(iv)	(((iv) >> 30) & 0x03)

#define MAX_MCS_INDEX	76

struct data_hdr_t{
    uint16_t	frame_ctl;
	uint16_t	duration;
	uint8_t	    da[ETH_ALEN];
	uint8_t	    sa[ETH_ALEN];
    uint8_t	    ra[ETH_ALEN];
    uint8_t	    ta[ETH_ALEN];
	uint8_t	    bssid[ETH_ALEN];
	uint16_t	ssc;
};

typedef enum{
	MGMT_HDR,
	DATA_HDR,
	CTRL_HDR,
}en_hdr;


#define GET_U_1(p)    ((uint8_t)(*(p)))
#define GET_S_1(p)    ((int8_t*)(*(p)))
#define GET_LE_U_2(p) \
	((uint16_t)(((uint16_t)(*((const uint8_t *)(p) + 1)) << 8) | \
	            ((uint16_t)(*((const uint8_t *)(p) + 0)) << 0)))

#define GET_LE_U_4(p) \
	((uint32_t)(((uint32_t)(*((const uint8_t *)(p) + 3)) << 24) | \
	            ((uint32_t)(*((const uint8_t *)(p) + 2)) << 16) | \
	            ((uint32_t)(*((const uint8_t *)(p) + 1)) << 8) | \
	            ((uint32_t)(*((const uint8_t *)(p) + 0)) << 0)))

#define	roundup2(x, y)	(((x)+((uint32_t)((y)-1)))&(~((uint32_t)((y)-1))))

/*
 * The radio capture header precedes the 802.11 header.
 *
 * Note well: all radiotap fields are little-endian.
 */
struct ieee80211_radiotap_header {
	uint8_t		it_version;	/* Version 0. Only increases
					 * for drastic changes,
					 * introduction of compatible
					 * new fields does not count.
					 */
	uint8_t		it_pad;
	uint16_t	it_len;		/* length of the whole
					 * header in bytes, including
					 * it_version, it_pad,
					 * it_len, and data fields.
					 */
	uint32_t	it_present;	/* A bitmap telling which
					 * fields are present. Set bit 31
					 * (0x80000000) to extend the
					 * bitmap by another 32 bits.
					 * Additional extensions are made
					 * by setting bit 31.
					 */
};

/* Name                                 Data type       Units
 * ----                                 ---------       -----
 *
 * IEEE80211_RADIOTAP_TSFT              uint64_t       microseconds
 *
 *      Value in microseconds of the MAC's 64-bit 802.11 Time
 *      Synchronization Function timer when the first bit of the
 *      MPDU arrived at the MAC. For received frames, only.
 *
 * IEEE80211_RADIOTAP_CHANNEL           2 x uint16_t   MHz, bitmap
 *
 *      Tx/Rx frequency in MHz, followed by flags (see below).
 *	Note that IEEE80211_RADIOTAP_XCHANNEL must be used to
 *	represent an HT channel as there is not enough room in
 *	the flags word.
 *
 * IEEE80211_RADIOTAP_FHSS              uint16_t       see below
 *
 *      For frequency-hopping radios, the hop set (first byte)
 *      and pattern (second byte).
 *
 * IEEE80211_RADIOTAP_RATE              uint8_t        500kb/s or index
 *
 *      Tx/Rx data rate.  If bit 0x80 is set then it represents an
 *	an MCS index and not an IEEE rate.
 *
 * IEEE80211_RADIOTAP_DBM_ANTSIGNAL     int8_t         decibels from
 *                                                     one milliwatt (dBm)
 *
 *      RF signal power at the antenna, decibel difference from
 *      one milliwatt.
 *
 * IEEE80211_RADIOTAP_DBM_ANTNOISE      int8_t         decibels from
 *                                                     one milliwatt (dBm)
 *
 *      RF noise power at the antenna, decibel difference from one
 *      milliwatt.
 *
 * IEEE80211_RADIOTAP_DB_ANTSIGNAL      uint8_t        decibel (dB)
 *
 *      RF signal power at the antenna, decibel difference from an
 *      arbitrary, fixed reference.
 *
 * IEEE80211_RADIOTAP_DB_ANTNOISE       uint8_t        decibel (dB)
 *
 *      RF noise power at the antenna, decibel difference from an
 *      arbitrary, fixed reference point.
 *
 * IEEE80211_RADIOTAP_LOCK_QUALITY      uint16_t       unitless
 *
 *      Quality of Barker code lock. Unitless. Monotonically
 *      nondecreasing with "better" lock strength. Called "Signal
 *      Quality" in datasheets.  (Is there a standard way to measure
 *      this?)
 *
 * IEEE80211_RADIOTAP_TX_ATTENUATION    uint16_t       unitless
 *
 *      Transmit power expressed as unitless distance from max
 *      power set at factory calibration.  0 is max power.
 *      Monotonically nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DB_TX_ATTENUATION uint16_t       decibels (dB)
 *
 *      Transmit power expressed as decibel distance from max power
 *      set at factory calibration.  0 is max power.  Monotonically
 *      nondecreasing with lower power levels.
 *
 * IEEE80211_RADIOTAP_DBM_TX_POWER      int8_t         decibels from
 *                                                     one milliwatt (dBm)
 *
 *      Transmit power expressed as dBm (decibels from a 1 milliwatt
 *      reference). This is the absolute power level measured at
 *      the antenna port.
 *
 * IEEE80211_RADIOTAP_FLAGS             uint8_t        bitmap
 *
 *      Properties of transmitted and received frames. See flags
 *      defined below.
 *
 * IEEE80211_RADIOTAP_ANTENNA           uint8_t        antenna index
 *
 *      Unitless indication of the Rx/Tx antenna for this packet.
 *      The first antenna is antenna 0.
 *
 * IEEE80211_RADIOTAP_RX_FLAGS          uint16_t       bitmap
 *
 *     Properties of received frames. See flags defined below.
 *
 * IEEE80211_RADIOTAP_XCHANNEL          uint32_t       bitmap
 *					uint16_t       MHz
 *					uint8_t        channel number
 *					uint8_t        .5 dBm
 *
 *	Extended channel specification: flags (see below) followed by
 *	frequency in MHz, the corresponding IEEE channel number, and
 *	finally the maximum regulatory transmit power cap in .5 dBm
 *	units.  This property supersedes IEEE80211_RADIOTAP_CHANNEL
 *	and only one of the two should be present.
 *
 * IEEE80211_RADIOTAP_MCS		uint8_t        known
 *					uint8_t        flags
 *					uint8_t        mcs
 *
 *	Bitset indicating which fields have known values, followed
 *	by bitset of flag values, followed by the MCS rate index as
 *	in IEEE 802.11n.
 *
 *
 * IEEE80211_RADIOTAP_AMPDU_STATUS	u32, u16, u8, u8	unitless
 *
 *	Contains the AMPDU information for the subframe.
 *
 * IEEE80211_RADIOTAP_VHT	u16, u8, u8, u8[4], u8, u8, u16
 *
 *	Contains VHT information about this frame.
 *
 * IEEE80211_RADIOTAP_VENDOR_NAMESPACE
 *					uint8_t  OUI[3]
 *                                      uint8_t        subspace
 *                                      uint16_t       length
 *
 *     The Vendor Namespace Field contains three sub-fields. The first
 *     sub-field is 3 bytes long. It contains the vendor's IEEE 802
 *     Organizationally Unique Identifier (OUI). The fourth byte is a
 *     vendor-specific "namespace selector."
 *
 */
enum ieee80211_radiotap_type {
	IEEE80211_RADIOTAP_TSFT = 0,
	IEEE80211_RADIOTAP_FLAGS = 1,
	IEEE80211_RADIOTAP_RATE = 2,
	IEEE80211_RADIOTAP_CHANNEL = 3,
	IEEE80211_RADIOTAP_FHSS = 4,
	IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
	IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
	IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
	IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
	IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
	IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
	IEEE80211_RADIOTAP_ANTENNA = 11,
	IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
	IEEE80211_RADIOTAP_DB_ANTNOISE = 13,
	IEEE80211_RADIOTAP_RX_FLAGS = 14,
	/* NB: gap for netbsd definitions */
	IEEE80211_RADIOTAP_XCHANNEL = 18,
	IEEE80211_RADIOTAP_MCS = 19,
	IEEE80211_RADIOTAP_AMPDU_STATUS = 20,
	IEEE80211_RADIOTAP_VHT = 21,
	IEEE80211_RADIOTAP_NAMESPACE = 29,
	IEEE80211_RADIOTAP_VENDOR_NAMESPACE = 30,
	IEEE80211_RADIOTAP_EXT = 31
};

/* channel attributes */
#define	IEEE80211_CHAN_TURBO	0x00010	/* Turbo channel */
#define	IEEE80211_CHAN_CCK	0x00020	/* CCK channel */
#define	IEEE80211_CHAN_OFDM	0x00040	/* OFDM channel */
#define	IEEE80211_CHAN_2GHZ	0x00080	/* 2 GHz spectrum channel. */
#define	IEEE80211_CHAN_5GHZ	0x00100	/* 5 GHz spectrum channel */
#define	IEEE80211_CHAN_PASSIVE	0x00200	/* Only passive scan allowed */
#define	IEEE80211_CHAN_DYN	0x00400	/* Dynamic CCK-OFDM channel */
#define	IEEE80211_CHAN_GFSK	0x00800	/* GFSK channel (FHSS PHY) */
#define	IEEE80211_CHAN_GSM	0x01000	/* 900 MHz spectrum channel */
#define	IEEE80211_CHAN_STURBO	0x02000	/* 11a static turbo channel only */
#define	IEEE80211_CHAN_HALF	0x04000	/* Half rate channel */
#define	IEEE80211_CHAN_QUARTER	0x08000	/* Quarter rate channel */
#define	IEEE80211_CHAN_HT20	0x10000	/* HT 20 channel */
#define	IEEE80211_CHAN_HT40U	0x20000	/* HT 40 channel w/ ext above */
#define	IEEE80211_CHAN_HT40D	0x40000	/* HT 40 channel w/ ext below */

/* Useful combinations of channel characteristics, borrowed from Ethereal */
#define IEEE80211_CHAN_A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define IEEE80211_CHAN_B \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_CCK)
#define IEEE80211_CHAN_G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN)
#define IEEE80211_CHAN_TA \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_TURBO)
#define IEEE80211_CHAN_TG \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN  | IEEE80211_CHAN_TURBO)


/* For IEEE80211_RADIOTAP_FLAGS */
#define	IEEE80211_RADIOTAP_F_CFP	0x01	/* sent/received
						 * during CFP
						 */
#define	IEEE80211_RADIOTAP_F_SHORTPRE	0x02	/* sent/received
						 * with short
						 * preamble
						 */
#define	IEEE80211_RADIOTAP_F_WEP	0x04	/* sent/received
						 * with WEP encryption
						 */
#define	IEEE80211_RADIOTAP_F_FRAG	0x08	/* sent/received
						 * with fragmentation
						 */
#define	IEEE80211_RADIOTAP_F_FCS	0x10	/* frame includes FCS */
#define	IEEE80211_RADIOTAP_F_DATAPAD	0x20	/* frame has padding between
						 * 802.11 header and payload
						 * (to 32-bit boundary)
						 */
#define	IEEE80211_RADIOTAP_F_BADFCS	0x40	/* does not pass FCS check */

/* For IEEE80211_RADIOTAP_RX_FLAGS */
#define IEEE80211_RADIOTAP_F_RX_BADFCS	0x0001	/* frame failed crc check */
#define IEEE80211_RADIOTAP_F_RX_PLCP_CRC	0x0002	/* frame failed PLCP CRC check */

/* For IEEE80211_RADIOTAP_MCS known */
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_KNOWN		0x01
#define IEEE80211_RADIOTAP_MCS_MCS_INDEX_KNOWN		0x02	/* MCS index field */
#define IEEE80211_RADIOTAP_MCS_GUARD_INTERVAL_KNOWN	0x04
#define IEEE80211_RADIOTAP_MCS_HT_FORMAT_KNOWN		0x08
#define IEEE80211_RADIOTAP_MCS_FEC_TYPE_KNOWN		0x10
#define IEEE80211_RADIOTAP_MCS_STBC_KNOWN		0x20
#define IEEE80211_RADIOTAP_MCS_NESS_KNOWN		0x40
#define IEEE80211_RADIOTAP_MCS_NESS_BIT_1		0x80

/* For IEEE80211_RADIOTAP_MCS flags */
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_MASK	0x03
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_20	0
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_40	1
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_20L	2
#define IEEE80211_RADIOTAP_MCS_BANDWIDTH_20U	3
#define IEEE80211_RADIOTAP_MCS_SHORT_GI		0x04 /* short guard interval */
#define IEEE80211_RADIOTAP_MCS_HT_GREENFIELD	0x08
#define IEEE80211_RADIOTAP_MCS_FEC_LDPC		0x10
#define IEEE80211_RADIOTAP_MCS_STBC_MASK	0x60
#define		IEEE80211_RADIOTAP_MCS_STBC_1	1
#define		IEEE80211_RADIOTAP_MCS_STBC_2	2
#define		IEEE80211_RADIOTAP_MCS_STBC_3	3
#define IEEE80211_RADIOTAP_MCS_STBC_SHIFT	5
#define IEEE80211_RADIOTAP_MCS_NESS_BIT_0	0x80

/* For IEEE80211_RADIOTAP_AMPDU_STATUS */
#define IEEE80211_RADIOTAP_AMPDU_REPORT_ZEROLEN		0x0001
#define IEEE80211_RADIOTAP_AMPDU_IS_ZEROLEN		0x0002
#define IEEE80211_RADIOTAP_AMPDU_LAST_KNOWN		0x0004
#define IEEE80211_RADIOTAP_AMPDU_IS_LAST		0x0008
#define IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_ERR		0x0010
#define IEEE80211_RADIOTAP_AMPDU_DELIM_CRC_KNOWN	0x0020

/* For IEEE80211_RADIOTAP_VHT known */
#define IEEE80211_RADIOTAP_VHT_STBC_KNOWN			0x0001
#define IEEE80211_RADIOTAP_VHT_TXOP_PS_NA_KNOWN			0x0002
#define IEEE80211_RADIOTAP_VHT_GUARD_INTERVAL_KNOWN		0x0004
#define IEEE80211_RADIOTAP_VHT_SGI_NSYM_DIS_KNOWN		0x0008
#define IEEE80211_RADIOTAP_VHT_LDPC_EXTRA_OFDM_SYM_KNOWN	0x0010
#define IEEE80211_RADIOTAP_VHT_BEAMFORMED_KNOWN			0x0020
#define IEEE80211_RADIOTAP_VHT_BANDWIDTH_KNOWN			0x0040
#define IEEE80211_RADIOTAP_VHT_GROUP_ID_KNOWN			0x0080
#define IEEE80211_RADIOTAP_VHT_PARTIAL_AID_KNOWN		0x0100

/* For IEEE80211_RADIOTAP_VHT flags */
#define IEEE80211_RADIOTAP_VHT_STBC			0x01
#define IEEE80211_RADIOTAP_VHT_TXOP_PS_NA		0x02
#define IEEE80211_RADIOTAP_VHT_SHORT_GI			0x04
#define IEEE80211_RADIOTAP_VHT_SGI_NSYM_M10_9		0x08
#define IEEE80211_RADIOTAP_VHT_LDPC_EXTRA_OFDM_SYM	0x10
#define IEEE80211_RADIOTAP_VHT_BEAMFORMED		0x20

#define IEEE80211_RADIOTAP_VHT_BANDWIDTH_MASK	0x1f

#define IEEE80211_RADIOTAP_VHT_NSS_MASK		0x0f
#define IEEE80211_RADIOTAP_VHT_MCS_MASK		0xf0
#define IEEE80211_RADIOTAP_VHT_MCS_SHIFT	4

#define IEEE80211_RADIOTAP_CODING_LDPC_USERn			0x01

#define	IEEE80211_CHAN_FHSS \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_GFSK)
#define	IEEE80211_CHAN_A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_B \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_CCK)
#define	IEEE80211_CHAN_PUREG \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN)

#define	IS_CHAN_FHSS(flags) \
	((flags & IEEE80211_CHAN_FHSS) == IEEE80211_CHAN_FHSS)
#define	IS_CHAN_A(flags) \
	((flags & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)
#define	IS_CHAN_B(flags) \
	((flags & IEEE80211_CHAN_B) == IEEE80211_CHAN_B)
#define	IS_CHAN_PUREG(flags) \
	((flags & IEEE80211_CHAN_PUREG) == IEEE80211_CHAN_PUREG)
#define	IS_CHAN_G(flags) \
	((flags & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)
#define	IS_CHAN_ANYG(flags) \
	(IS_CHAN_PUREG(flags) || IS_CHAN_G(flags))

#define MCS_INFO_LEN		100

struct radio_stamp{
	uint32_t freq;
	char mcs_info[MCS_INFO_LEN];
};

class ieee80211_parse{
private:
	uint32_t iv;
	
	int parse_elements(struct mgmt_body_t *mgmt_body, const uint8_t *p, int offset, uint32_t length);
    int handle_beacon(const uint8_t *p, uint32_t length);
    int handle_assoc_request(const uint8_t *p, uint32_t length);
    int handle_assoc_response(const uint8_t *p, uint32_t length);
    int handle_reassoc_request(const uint8_t *p, uint32_t length);
    int handle_reassoc_response(const uint8_t *p, uint32_t length);
    int handle_probe_request(const uint8_t *p, uint32_t length);
    int handle_probe_response(const uint8_t *p, uint32_t length);
    int handle_atim(void);
    int handle_disassoc(const uint8_t *p, uint32_t length);
    int handle_auth(const uint8_t *p, uint32_t length);
    int handle_deauth(const uint8_t *src, const uint8_t *p, uint32_t length);
    int handle_action(const uint8_t *src, const uint8_t *p, uint32_t length);
    int wep_parse(const uint8_t *p);
    int mgmt_body_parse(uint16_t fc, const uint8_t *src, const uint8_t *p, uint32_t length);
    int ctrl_body_parse(uint16_t fc, const uint8_t *p);
    void get_data_src_dst_mac(uint16_t fc, const uint8_t *p, const uint8_t **srcp, const uint8_t **dstp);
    void get_mgmt_src_dst_mac(const uint8_t *p, const uint8_t **srcp, const uint8_t **dstp);
    void data_header_parse(uint16_t fc, const uint8_t *p);
	void mgmt_header_parse(const uint8_t *p);
	void ctrl_header_parse(uint16_t fc, const uint8_t *p);
	int extract_header_length(uint16_t fc);
	int extract_mesh_header_length(const uint8_t *p);
	void ieee_802_11_hdr_parse(uint16_t fc, const uint8_t *p, uint32_t hdrlen, uint32_t meshdrlen);
	uint32_t ieee802_11_parse(const uint8_t *p, uint32_t length, uint32_t orig_caplen, int pad, uint32_t fcslen);
public:
	uint16_t parse_fc;
	const uint8_t* mac_src;
	const uint8_t* mac_dst;
	struct radio_stamp radio;
	struct {
		en_hdr hdr_type;
		union{
			struct {
				en_ctrl_hdr ctrl_hdr_type;
				union {
					ctrl_bar_hdr_t bar;
					ctrl_ba_hdr_t ba;
					ctrl_ps_poll_hdr_t ps_poll;
					ctrl_rts_hdr_t rts;
					ctrl_cts_hdr_t cts;
					ctrl_ack_hdr_t ack;
					ctrl_end_hdr_t end;
					ctrl_end_ack_hdr_t end_ack;
				}hdr;
			}ctrl_hdr;
			struct data_hdr_t data_hdr;
			struct mgmt_header_t mgmt_hdr;
		}hdr;
	}hdr;

	struct{
		struct mgmt_body_t mgmt_body;
	}body;

	void ieee802_11_if_parse(const struct pcap_pkthdr *h, const uint8_t *p);
	// radio stamp is not completely inplemented, do not use
	void parse_chaninfo(uint16_t freq, uint32_t flags, uint32_t presentflags);
	void print_header_type();
};

} // namespace ieee80211_parse
#ifdef __cplusplus
}
#endif