#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include "mlo_pack.h"
#include "send.h"
#include "json_wrapper.h"
#include "pack_cap.h"
#include "link_monitor.h"

FILE *logfile;
extern struct link_stat_buffer lstat_buffer;
void help(){
    printf("usage:\n");
    printf("./sta_send json_file\n");
}

     
int main(int argc, char** argv){
    if(argc < 2){
        printf("%d\n", argc);
        help();
        return 0;
    }

    srand(getpid());

    cJSON* json = parse_json(argv[1]);

    mcs_status_t mcs;

    write_stream stream(json);
    int if_cnt = stream.getifcnt();

    pkcap_resolve pkcap[2];
    interface_status_t* if_staus = (interface_status_t*)malloc(sizeof(interface_status_t));
    
    for(int i = 0; i < if_cnt; ++i){
        get_monitor_ifstatus(&if_staus[i], json, i);
        pkcap[i].init(&stream.sub_stream[i].setting.local_addr);
    }

    struct link_stat_buffer lstat_buffer;
    link_monitor_init(&lstat_buffer);
    char filter_buf[256];
    for(int i = 0; i < if_cnt; ++i){
        get_mcs_status(&mcs, json, i);
        nl80211_set_mcs(&mcs, pkcap[i].dev_name);
        register_dev(&lstat_buffer, pkcap[i].dev_name);
        get_monitor_filter(json, i, filter_buf);
        pkcap[i].add_mon_if(&if_staus[i], filter_buf);
        pkcap[i].start(-1);
    }

    sleep(1);
    sta_info pre_info[2];
    for(int i = 0; i < 2; ++i){
        memcpy(&pre_info[i], &lstat_buffer.l_stat[i], sizeof(pre_info[i]));
    }
    cJSON_Delete(json);
    stream.start();

    uint8_t done = 0;
    while(!stream.is_alldone()){
        for(int i = 0; i < if_cnt; ++i){
            pkcap[i].print_packqueue(&lstat_buffer.l_stat[i]);
        }
    }
    sleep(10);
    for(int i = 0; i < if_cnt; ++i){
        pkcap[i].print_packqueue(&lstat_buffer.l_stat[i]);
        pkcap[i].~pkcap_resolve();
    }
    printf("total send %d packs\n", stream.pack_cnt);
    for(int i = 0; i < if_cnt; ++i){
        printf("%s tx situation\n", pkcap[i].dev_name);
        sta_info* info = &lstat_buffer.l_stat[i];
        printf("tx packets:%10d\ntx retries:%10d\ntx failed:%10d\n", 
        info->tx_packets-pre_info[i].tx_packets, 
        info->tx_retrys-pre_info[i].tx_retrys, 
        info->tx_failed-pre_info[i].tx_failed);
    }
    exit(0);
}