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
#include <signal.h>

int exit_signal = 0;
void help(){
    printf("usage:\n");
    printf("./sta_send json_file\n");
}

void signalcallback(int n){
    exit_signal = 1;
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
    int if_cnt = get_if_cnt(json);

    pkcap_resolve pkcap[2];
    interface_status_t if_staus[2];
    
    char filter_buf[1024];
    printf("begin to init device \n");
    printf("if_cnt:%d\n", if_cnt);
    for(int i = 0; i < if_cnt; ++i){
        get_monitor_ifstatus(&if_staus[i], json, i);
        printf("init %s\n", if_staus[i].name);
        get_monitor_filter(json, i, filter_buf);
        pkcap[i].init((char*)if_staus[i].name, (char*)filter_buf);
    }

    for(int i = 0; i < if_cnt; ++i){
        pkcap[i].start(-1);
    }

    signal(SIGINT, signalcallback);

    sleep(1);
    cJSON_Delete(json);

    uint8_t done = 0;
    while(exit_signal == 0){
        for(int i = 0; i < if_cnt; ++i){
            pkcap[i].print_packqueue(NULL);
        }
    }
    exit(0);
}