#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "mlo_pack.h"
#include "pack_cap.h"
#include "mlo_time.h"
#include "link_monitor.h"
#include "json_wrapper.h"

#define SIZE_ETHERNET                   14
#define SIZE_UDP                        8

using namespace ieee80211_parser;

mlo_pack* pkcap_resolve::resolve_ctr_frame(const ieee80211_parse& parser, const struct pcap_pkthdr * header, char* info_buf, int buf_len){
    uint64_t ack_bitmap;
    uint16_t start_sn;
    mlo_pack* back_pack = NULL;
    switch (parser.hdr.hdr.ctrl_hdr.ctrl_hdr_type){
        case BA_HDR:
            ack_bitmap = parser.hdr.hdr.ctrl_hdr.hdr.ba.bitmap;
            snprintf(info_buf, buf_len, "BA,%lx", ack_bitmap);
            start_sn = SSC_SN(parser.hdr.hdr.ctrl_hdr.hdr.ba.ssc);
            back_pack = init_pack(start_sn, (uint8_t*)info_buf, buf_len);
            back_pack->tstamp.tv_sec = header->ts.tv_sec;
            back_pack->tstamp.tv_usec = header->ts.tv_usec;
            back_pack->sub_seq = CSV_BA;
            break;
        case BAR_HDR:
            snprintf(info_buf, buf_len, "BAR");
            start_sn = SSC_SN(parser.hdr.hdr.ctrl_hdr.hdr.bar.seq);
            back_pack = init_pack(start_sn, (uint8_t*)info_buf, buf_len);
            back_pack->tstamp.tv_sec = header->ts.tv_sec;
            back_pack->tstamp.tv_usec = header->ts.tv_usec;
            back_pack->sub_seq = CSV_BA;
        default:
            break;
    }
    return back_pack;
}

mlo_pack* pkcap_resolve::resolve_data_frame(const ieee80211_parse& parser, const struct pcap_pkthdr * header, const uint8_t* payload, char* info_buf, int buf_len){
    mlo_pack* back_pack = NULL;
    uint16_t start_sn;
    const uint8_t* pack_data = NULL;
    if(header->caplen < PACKET_PAYLOAD_LEN+PATLOAD_OFFSET){
        snprintf(info_buf, buf_len, "DATA,unkown data");
        return back_pack;
    }
    else {
        pack_data = payload+(header->len-(PACKET_PAYLOAD_LEN+PATLOAD_OFFSET)); 
        int seq = 0, sub_seq = 0;
        memcpy(&seq, pack_data+SEQ_OFFSET, sizeof(seq));
        memcpy(&sub_seq, pack_data+SUBSEQ_OFFSET, sizeof(sub_seq));
        seq = ntohl(seq);
        sub_seq = ntohl(sub_seq);
        snprintf(info_buf, buf_len, "DATA,%d,%d", seq, sub_seq);
        start_sn = SSC_SN(parser.hdr.hdr.data_hdr.ssc);
        back_pack = init_pack(start_sn, (uint8_t*)info_buf, buf_len);
        back_pack->tstamp.tv_sec = header->ts.tv_sec;
        back_pack->tstamp.tv_usec = header->ts.tv_usec;
        back_pack->sub_seq = CSV_DATA;
    }
    return back_pack;
}

/**
 * @brief pcap callback
 * If the NIC is in managed mode, packets are ethernet frames
 * If the NIC is in monitor mode, packets are 80211 wireless frames
 * 
 * **/
void cap_callback(uint8_t *args, const struct pcap_pkthdr * header, const uint8_t * pack){
    pkcap_resolve* pkcap = (pkcap_resolve*)args;
#ifdef SNIFF_80211
    mlo_pack* info_pack = pkcap->pack_parse(header, pack);
    if(info_pack != NULL)
        insert_pack_tail(pkcap->head, info_pack);
    pcap_dump((uint8_t*)pkcap->pdump, header, pack);
#endif
    ++pkcap->pack_cnt;
}

void* pkcap_routine_entry(void* args){
    pkcap_resolve* pkcap = (pkcap_resolve*)args;
    printf("start capture on %s\n", pkcap->mon_name);
    pkcap->loop_cap();
    return NULL;
}

/*****
* @brief start a capture routine
* @param cap_cnt: how many packets the route capture.  
* A value of -1 or 0 for cap_cnt is equivalent to infinity
****/
void pkcap_resolve::start(int cap_cnt){
    handle = pcap_open_live(mon_name, MTU_SIZE, 0, 0, errbuf);
    if(handle == NULL){
        fprintf(stderr, "pcap error:%s\n", errbuf);
    }

    char dump_file_name[255];
    snprintf(dump_file_name, 255, "./csv/%s.pcap", mon_name);
    pdump = pcap_dump_open(handle, dump_file_name);

    struct bpf_program bpf_fp;
    memset(&bpf_fp, 0, sizeof(bpf_fp));
    bpf_u_int32 netmask, net;
    int status;
    status = pcap_lookupnet(mon_name, &net, &netmask, errbuf);
    status = pcap_compile(handle, &bpf_fp, bpf_filter, 1, netmask);
    if(status != 0){
        pcap_perror(handle, "error in pcap_compile");
    }

    pcap_setfilter(handle, &bpf_fp);
    cap_total = cap_cnt;
    pthread_create(&cap_tid, NULL, pkcap_routine_entry, this);
}

void pkcap_resolve::dev_init(struct sockaddr_in* addr){
    char ip[20];
    printf("check addr %s\n",  inet_ntop(AF_INET, &addr->sin_addr.s_addr, ip, 20));
    int ret = 0;
    pcap_if_t* dev_if = NULL;

    ret = pcap_findalldevs(&dev_if, errbuf);
    if(ret != 0){
        pcap_perror(handle, "pcap error:");
    }
    pcap_if_t* devif_ptr = dev_if;
    for(devif_ptr = dev_if; devif_ptr; devif_ptr = devif_ptr->next){
        pcap_addr_t *addr_ptr = devif_ptr->addresses;
        while(addr_ptr != NULL&&addr_ptr->addr->sa_family != AF_INET){
            addr_ptr = addr_ptr->next;
        }
        if(addr_ptr == NULL) continue;
        struct sockaddr_in* addr_in = (struct sockaddr_in*)addr_ptr->addr;
        if(addr_in->sin_addr.s_addr == addr->sin_addr.s_addr){
            strcpy(dev_name, devif_ptr->name);
            break;
        }
    }
    printf("find %s, local IP:%s\n", dev_name, inet_ntop(AF_INET, &addr->sin_addr.s_addr, ip, 20));
}

void pkcap_resolve::init(struct sockaddr_in* addr){
    dev_init(addr);
    cap_done = 0;
    pack_cnt = 0;
    head = init_pack_queue();
    
    memset(dev_mac, 0, sizeof(dev_mac));
    nl80211_get_mac(dev_mac, dev_name);
}

int pkcap_resolve::add_mon_if(interface_status_t* if_status, char* filter){
    printf("add a virtual interface %s to %s\n", if_status->name, dev_name);
    nl80211_add_interface(if_status, dev_name);
    strcpy(mon_name, (char*)if_status->name);
    char mac_a[50];
    macaddr_ntoa(mac_a, dev_mac);

    char name[50];

    int len = sizeof(csv)/sizeof(*csv);
    for(int i = 0; i < len; ++i){
        snprintf(name, 50, "./csv/%s_%s.csv", csv_file_prefix_tok[i].s, mon_name);
        csv[i] = fopen(name, "w");
        fprintf(csv[i], csv_head_tok[i].s);
    }
    strncpy(bpf_filter, filter, sizeof(bpf_filter));
    printf("%s filter:%s\n", mon_name, bpf_filter);
    return 1;
}

pkcap_resolve::pkcap_resolve(){
    handle = NULL;
    head = NULL;
    int len = sizeof(csv)/sizeof(*csv);
    for(int i = 0; i < len; ++i){
        csv[i] = NULL;
    }
    pack_cnt = 0;
    pdump = NULL;
}

pkcap_resolve::pkcap_resolve(struct sockaddr_in* addr){
    init(addr);
}

pkcap_resolve::pkcap_resolve(char* mon_dev){
    strcpy(mon_name, mon_dev);

}


pkcap_resolve::~pkcap_resolve(){
    int len = sizeof(csv)/sizeof(*csv);
    for(int i = 0; i < len; ++i){
        fclose(csv[i]);
    }
    pcap_close(handle);
    destory_pack_queue(head);
    nl80211_del_interface(mon_name);
    pcap_dump_close(pdump);
}

void pkcap_resolve::print_pack(mlo_pack* pack, struct sta_info* sta_stat){
    uint64_t us = pack->tstamp.tv_sec*1000000+pack->tstamp.tv_usec;
    if(sta_stat != NULL){
        fprintf(csv[pack->sub_seq], "%d,%lu,%lu,%s,%s\n", 
                pack->seq, 
                us,
                sta_stat->tx_rate.tx_rate,
                sta_stat->tx_rate.mcs_info,
                pack->payload
        );
    }
    else {
        fprintf(csv[pack->sub_seq], "%d,%lu,%lu,%s,%s\n", 
                pack->seq, 
                us,
                0,
                "",
                pack->payload
        );
    }
}

void pkcap_resolve::print_packqueue(struct sta_info* sta_stat){
    int cnt = 0;
    while(!pack_queue_empty(head) && cnt < 100){
        mlo_pack* info_pack = remove_pack_first(head);
        if(info_pack != NULL){
            print_pack(info_pack, sta_stat);
            destory_pack(info_pack);
        }
        cnt = cnt+1;
    }
}

void pkcap_resolve::loop_cap(){
    pcap_loop(handle, cap_total, cap_callback, (uint8_t*)this);
    pcap_close(handle);
}

mlo_pack* pkcap_resolve::pack_parse(mlo_pack* pack){
    char info[100];
    struct pcap_pkthdr header;
    header.ts.tv_sec = pack->tstamp.tv_sec;
    header.ts.tv_usec = pack->tstamp.tv_usec;
    const struct ieee80211_radiotap_header *hdr;
    hdr = (const struct ieee80211_radiotap_header *)pack->payload;
    uint32_t len = GET_LE_U_2(&(hdr->it_len));
    static ieee80211_parse parser;
    header.len = pack->payload_len;
    header.caplen = header.len;
    parser.ieee802_11_if_parse(&header, pack->payload+len);
    // parser.print_header_type();
    mlo_pack* new_pack = NULL;
    if(parser.hdr.hdr_type == CTRL_HDR){
        new_pack = resolve_ctr_frame(parser, &header, info, sizeof(info));
    }
    else if(parser.hdr.hdr_type == DATA_HDR){
        new_pack = resolve_data_frame(parser, &header, pack->payload, info, sizeof(info));
    }
    return new_pack;
}

mlo_pack* pkcap_resolve::pack_parse(const struct pcap_pkthdr * header, const uint8_t * pack){
    char info[100];
    memset(info, 0, sizeof(info));
    const struct ieee80211_radiotap_header *hdr;
    hdr = (const struct ieee80211_radiotap_header *)pack;
    uint32_t len = GET_LE_U_2(&(hdr->it_len));
    static ieee80211_parse parser;
    parser.ieee802_11_if_parse(header, pack+len);
    // parser.print_header_type();
    mlo_pack* new_pack = NULL;
    if(parser.hdr.hdr_type == CTRL_HDR){
        new_pack = resolve_ctr_frame(parser, header, info, sizeof(info));
    }
    else if(parser.hdr.hdr_type == DATA_HDR){
        new_pack = resolve_data_frame(parser, header, pack, info, sizeof(info));
    }
    return new_pack;
}


void pkcap_resolve::init(char* mon, char* filter){
    cap_done = 0;
    pack_cnt = 0;
    head = init_pack_queue();
    strcpy(mon_name, (char*)mon);

    char name[50];

    int len = sizeof(csv)/sizeof(*csv);
    for(int i = 0; i < len; ++i){
        snprintf(name, 50, "./csv/%s_%s.csv", csv_file_prefix_tok[i].s, mon_name);
        csv[i] = fopen(name, "w");
        fprintf(csv[i], csv_head_tok[i].s);
    }
    strncpy(bpf_filter, filter, sizeof(bpf_filter));
    printf("%s filter:%s\n", mon_name, bpf_filter);
}