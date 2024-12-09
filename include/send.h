#pragma once

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include "mlo_pack.h"
#include "pack_cap.h"
#include "json_wrapper.h"
#include "mlo_time.h"

#ifdef __cplusplus
extern "C" {
#endif

struct substream_setting{
    struct sockaddr_in      target_addr;
    struct sockaddr_in      local_addr;
    int                     sockfd;
    double                  bitrate; 
};

struct write_sub_stream{
    pack_head*                          pack_bucket;
    pack_head*                          send_queue;
    struct substream_setting            setting;
    struct timeval                      write_time;
    int                                 subseq;
    uint8_t                             pack_done;
    uint8_t                             send_done;
    uint8_t                             control_done;
};

typedef struct write_sub_stream send_unit; 

class write_stream{
    friend void* pack_thread_entry(void* arg);


    pthread_t   read_tid; //read file
    int         if_cnt;
    double      bernoulli_p;
    double      if_p;
    pthread_t   send_tid;
    pthread_t   control_tid;
    uint32_t    time_slot;
public:
    send_unit*  sub_stream;
    int         pack_cnt;
    int         seq;
    FILE*       read_file;

    void init(cJSON* json);
    void start();
    int getifcnt();
    double get_bernoulli_p();
    double get_if_p();
    void resolve_callback(struct link_stat_buffer& lbuff);
    bool is_alldone();
    write_stream(cJSON* json);
    ~write_stream();
};

write_stream* init_write_stream(cJSON* json);
#ifdef __cplusplus
}
#endif