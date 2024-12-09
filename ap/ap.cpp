#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include "mlo_pack.h"
#include "json_wrapper.h"
#include "pack_cap.h"
#include "link_monitor.h"
#include "stream_init.h"

void help(){
    printf("usage:\n");
    printf("./ap_recv json_file\n");
}

int main(int argc, char** argv){
    if(argc < 2){
        help();
    }

    cJSON* json = parse_json(argv[1]);
   
    write_stream* stream = stream_init(json);

    printf("if_cnt:%d\n", stream->stream_cnt);

    while(1){
        ;
    }
    return 0;
}