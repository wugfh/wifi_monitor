#pragma once

#include <sys/queue.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "mlo_time.h"
#include "token_bucket.h"

#define PACKET_PAYLOAD_LEN      1400
#define PACK_QUEUE_CAPACITY     2000                        
#define MTU_SIZE                1500


#ifdef __cplusplus
extern "C" {
#endif

enum{
    SEQ_OFFSET          = 0,
    SUBSEQ_OFFSET       = 4,
    USEC_OFFSET         = 8,
    SEC_OFFSET          = 12,
    PAYLOAD_LEN_OFFSET  = 16,
    PATLOAD_OFFSET      = 20,
};

/**
 *  @brief
 * 
 * 
 * **/
struct packet_struct{
    int seq;
    int sub_seq;
    int payload_len;
    mlo_time_t tstamp;
    uint8_t* payload;

    TAILQ_ENTRY(packet_struct) pack_queue;
};

typedef struct packet_struct mlo_pack;

TAILQ_HEAD(queue_head, packet_struct);

/**
 *  @brief 
 * 
 * **/
struct pack_queue_head{
    struct queue_head head;
    int pack_cnt;
    pthread_mutex_t queue_lock;
};

typedef struct pack_queue_head pack_head;

mlo_pack* init_pack(int seq, const uint8_t* data, int len);
int destory_pack(mlo_pack* pack);
pack_head* init_pack_queue();
int destory_pack_queue(pack_head* queue_head);
mlo_pack* insert_pack_tail(pack_head* queue_head, mlo_pack* pack);
mlo_pack* init_pack_fromfile(int seq, FILE* fp);
mlo_pack* pack_queue_first(pack_head* queue_head);
int pack_queue_empty(pack_head* queue_head);
int delete_pack_first(pack_head* queue_head);
mlo_pack* insert_pack_order(int seq, pack_head* head, mlo_pack* target_pack);
int write_pack_file(mlo_pack* pack, FILE* fp);
mlo_pack* remove_pack_first(pack_head* queue_head);
int pack_queue_cnt(pack_head* head);
int packet2buf(mlo_pack* pack, uint8_t* send_buf, int sub_seq);

#ifdef __cplusplus
}
#endif