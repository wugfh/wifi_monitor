#pragma once

#include <pcap.h>
#include <net/ethernet.h>
#include "mlo_pack.h"
#include "link_monitor.h"
#include "ieee80211_parse.h"
#include "util.h"

#define DEV_NAME_SIZE       50
#define STA_CSV_HEAD		"frame_sn,time,rate,mcs,frame_type,seq,subseq\n"
#define AP_CSV_HEAD			"frame_sn,time,frame_type,seq,subseq\n"
#define FCS_LEN				4

/* Ethernet addresses are 6 bytes */
#ifdef __cplusplus
extern "C" {
#endif

/* Ethernet header */
struct sniff_ethernet {
	uint8_t ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
	uint8_t ether_shost[ETHER_ADDR_LEN]; /* Source host address */
	uint16_t ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
	uint8_t ip_vhl;		/* version << 4 | header length >> 2 */
	uint8_t ip_tos;		/* type of service */
	uint16_t ip_len;		/* total length */
	uint16_t ip_id;		/* identification */
	uint16_t ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* don't fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
	uint8_t ip_ttl;		/* time to live */
	uint8_t ip_p;		/* protocol */
	uint16_t ip_sum;		/* checksum */
	struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
typedef uint32_t tcp_seq;

struct sniff_tcp {
	uint16_t th_sport;	/* source port */
	uint16_t th_dport;	/* destination port */
	tcp_seq th_seq;		/* sequence number */
	tcp_seq th_ack;		/* acknowledgement number */
	uint8_t th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
	uint8_t th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	uint16_t th_win;		/* window */
	uint16_t th_sum;		/* checksum */
	uint16_t th_urp;		/* urgent pointer */
};

/* UDP header */
struct sniff_udp{
    uint16_t usport; /* source port */
    uint16_t udport; /* destination port */
    uint16_t ulen;  /* udp length */
    uint16_t sum;   /* udp checksum */
};

#define BPF_FILTER_BUFFER_LEN	255


void cap_callback(uint8_t *args, const struct pcap_pkthdr * header, const uint8_t * pack);
void* pkcap_routine_entry(void* args);

enum{
	CSV_DATA,
	CSV_BA,
};

const struct tok csv_head_tok[] = {
	{CSV_DATA, "frame_sn,time,rate,mcs,frame_type,seq,subseq\n"},
	{CSV_BA, "frame_sn,time,rate,mcs,frame_type,ackbitmap\n"}
};

const struct tok csv_file_prefix_tok[] = {
	{CSV_DATA, "data"},
	{CSV_BA, "ba"}
};



class pkcap_resolve{
friend void cap_callback(uint8_t *args, const struct pcap_pkthdr * header, const uint8_t * pack);
friend void* pkcap_routine_entry(void* args);
private:
	pcap_t* handle;
	pcap_dumper_t* pdump;
	FILE* csv[sizeof(csv_file_prefix_tok)/sizeof(struct tok)];
	pack_head* head;
	pthread_t cap_tid;
	char bpf_filter[256];
    char errbuf[PCAP_ERRBUF_SIZE];
	uint8_t dev_mac[ETH_ALEN];
	uint32_t max_pcnt;
	int cap_total;
	void dev_init(struct sockaddr_in* addr);
	mlo_pack* resolve_ctr_frame(const ieee80211_parser::ieee80211_parse& parser, const struct pcap_pkthdr * header, char* info_buf, int buf_len);
	mlo_pack* resolve_data_frame(const ieee80211_parser::ieee80211_parse& parser, const struct pcap_pkthdr * header, const uint8_t* payload, char* info_buf, int buf_len);
	void loop_cap();
public:
    char dev_name[DEV_NAME_SIZE];
	char mon_name[BPF_FILTER_BUFFER_LEN];
	uint8_t run_type;
	uint8_t cap_done;
	uint32_t pack_cnt;
	void print_packqueue(struct sta_info* lstat);
	void print_pack(mlo_pack* pack, struct sta_info*lstat);
	void start(int cap_cnt);
	int add_mon_if(interface_status_t* if_status, char* bpf_filter);
	void init(struct sockaddr_in* addr);
	void init(char* mon, char* filter);
	mlo_pack* pack_parse(mlo_pack* pack);
	mlo_pack* pack_parse(const struct pcap_pkthdr * header, const uint8_t * pack);
	pkcap_resolve();
    pkcap_resolve(struct sockaddr_in* addr);
	pkcap_resolve(char* mon_dev);
	~pkcap_resolve();
};

#ifdef __cplusplus
}
#endif
