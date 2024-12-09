#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <random>
#include "send.h"
#include "link_monitor.h"

#define TIMEOUT_US          (1000000)
#define PACK_CNT_LIMIT      (1000)

static int init_sockaddr(struct sockaddr_in* sock_addr, char* ip, uint16_t port){
    memset(sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port = htons(port);
    inet_pton(AF_INET, ip, &sock_addr->sin_addr);
    return 0;
}

static int creat_sta_sock(struct sockaddr_in* sock_addr, char* ip, uint16_t port){
    init_sockaddr(sock_addr, ip, port);
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int status = bind(sock_fd, (struct sockaddr*) sock_addr, sizeof(*sock_addr));
    if(status != 0){
        perror("error occurs in bind function");
    }
    return sock_fd;
}

static void* send_thread_entry(void* unit){
    send_unit* thread_unit = (send_unit*)unit;
    pthread_detach(pthread_self());
    int fd = thread_unit->setting.sockfd;
    pack_head* head = thread_unit->send_queue;
    struct sockaddr* ap_addr = (struct sockaddr*)(&thread_unit->setting.target_addr);
    uint8_t* send_buf = (uint8_t*)malloc(MTU_SIZE);
    mlo_pack* pack;
    uint32_t len = PACKET_PAYLOAD_LEN;
    while(len > PACKET_PAYLOAD_LEN/2){
        while(!pack_queue_empty(head)){
            pack = remove_pack_first(head);
            len = packet2buf(pack, send_buf, thread_unit->subseq);
            write(fd, send_buf, len);
            ++thread_unit->subseq;
            destory_pack(pack);
        }
    }
    char ip[20];
    printf("%s send done\n", inet_ntop(AF_INET, &thread_unit->setting.local_addr.sin_addr, ip, 20));
    thread_unit->send_done = 1;
    return NULL;
}

static int create_send_pthread(pthread_t* tid, send_unit* unit){
    int status = pthread_create(tid, NULL, send_thread_entry, unit);
    if(status != 0){
        perror("send thread create fail");
        return -1;
    }
    return 0;
}

static send_unit* init_send_unit(send_unit* unit_send, cJSON* json, int index){
    char* local_ip = get_local_ip(json, index);
    char* remote_ip = get_remote_ip(json, index);
    double rate = get_if_bitrate(json, index);
    uint16_t port = get_transfer_port(json);

    unit_send->send_queue = init_pack_queue();
    unit_send->pack_bucket = init_pack_queue();
    unit_send->setting.sockfd = creat_sta_sock(&unit_send->setting.local_addr, local_ip, port);
    unit_send->setting.bitrate = rate;
    unit_send->subseq = 0;
    unit_send->pack_done = 0;
    unit_send->send_done = 0;
    gettimeofday(&unit_send->write_time, NULL);
    init_sockaddr(&unit_send->setting.target_addr, remote_ip, port);
    connect(unit_send->setting.sockfd , (struct sockaddr*)(&unit_send->setting.target_addr), sizeof(struct sockaddr_in));
    return unit_send;
}

/**
 * @brief For flow distribution, if using non-Dynamic congestion-aware policies
 * recommended to be pre-allocated first, just like a function; if using dynamic congestion-aware policies
 * recommended runtime allocation, just like a thread
 * 
 * @param arg  must be write_stream* type, void * is only to be compatible with threaded mode
 * 
 * **/
void* pack_thread_entry(void* arg){
    write_stream* stream = (write_stream*)arg;
    mlo_pack* pack;
    uint8_t* data=(uint8_t*)malloc(PACKET_PAYLOAD_LEN);
    memset(data, 0xff, PACKET_PAYLOAD_LEN);
    std::geometric_distribution dis(stream->bernoulli_p);
    std::bernoulli_distribution if_dis(stream->if_p);
    printf("bernoulli 1 p :%f\n", stream->bernoulli_p);
    printf("interface 1 p :%f\n", stream->if_p);
    std::random_device seed1, seed2;
    std::mt19937 gen_if(seed1()), gen_time(seed2());
    FILE* time_fp = fopen("./csv/send_time.csv", "w");
    fprintf(time_fp, "seq,time\n");
    while((pack=init_pack(stream->seq, data, PACKET_PAYLOAD_LEN)) != NULL){
        useconds_t  t = dis(gen_time)*stream->time_slot;
        usleep(t);
        ++stream->seq;
        if(stream->seq > stream->pack_cnt){
            destory_pack(pack);
            break;
        }
        // n is to determine which interface shoule be used
        int n = if_dis(gen_if);
        gettimeofday(&pack->tstamp, NULL);
        uint64_t time_us = mlo_time2us(&pack->tstamp);
        insert_pack_tail(stream->sub_stream[n].send_queue, pack);
        fprintf(time_fp, "%d,%lu\n", stream->seq-1, time_us);
    }
    char ip[20];
    for(int i = 0; i < stream->if_cnt; ++i){
        ++stream->seq;
        pack = init_pack(stream->seq, NULL, 0);
        insert_pack_tail(stream->sub_stream[i].send_queue, pack);
        stream->sub_stream[i].pack_done = 1;
        printf("send %s end pack\n", inet_ntop(AF_INET, &stream->sub_stream[i].setting.local_addr.sin_addr, ip, 20));
    }

    printf("pack done\n");
    return 0;
}

// static void* rate_control_thread_entry(void* args){
//     send_unit* thread_unit = (send_unit*)args;
//     pack_head* bucket = thread_unit->pack_bucket;
//     pack_head* queue = thread_unit->send_queue;
//     while(!pack_queue_empty(bucket)||thread_unit->pack_done==0){
//         while(!pack_queue_empty(bucket)){
//             mlo_pack* pack = remove_pack_first(bucket);
//             gettimeofday(&pack->tstamp, NULL);
//             insert_pack_tail(queue, pack);
//         }
//     }
//     thread_unit->control_done = 1;
//     return NULL;
// }

void write_stream::init(cJSON* json){
    int interface_num = get_if_cnt(json);
    printf("interface num:%d\n", interface_num);
    if_cnt = interface_num;
    sub_stream = (send_unit*)malloc(sizeof(send_unit)*interface_num);
    read_file = fopen(get_src_file(json), "r");
    seq = 0;
    pack_cnt = get_pack_cnt(json);
    time_slot = get_time_slot(json);
    bernoulli_p = get_bernoulli_param(json);
    if_p = get_interface_probability(json);
    for(int i = 0; i < interface_num; ++i){
        init_send_unit(&sub_stream[i], json, i);
    }
}

void write_stream::start(){
    pthread_create(&read_tid, NULL, pack_thread_entry, this);
    for(int i = 0; i < if_cnt; ++i){
        // pthread_create(&control_tid, NULL, rate_control_thread_entry,  &sub_stream[i]);
        create_send_pthread(&send_tid, &sub_stream[i]);
    }
}

int write_stream::getifcnt(){
    return if_cnt;
}
  
double write_stream::get_bernoulli_p(){
    return bernoulli_p;
}

double write_stream::get_if_p(){
    return if_p;
}

write_stream::write_stream(cJSON* json){
    init(json);
}

write_stream::~write_stream(){
    free(sub_stream);
    fclose(read_file);
}


bool write_stream::is_alldone(){
    int cnt = 0;
    for(int i = 0; i < if_cnt; ++i){
        cnt += sub_stream[i].send_done;
    }
    if(cnt == if_cnt) return 1;
    else return 0;
}