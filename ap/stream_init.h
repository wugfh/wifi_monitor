#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mlo_pack.h"
#include "json_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief record sub-stream infomation.
 * @pack_queue: the queue packet will insert into. 
 * @target_addr: used to store remote address.
 * @local_addr: used to store local address.
 * @recv_tid: each sub-stream has a corresponding thread.
 * @sockfd: the socket file descriptor. every sub-stream has a socket file descriptor.
 * @sub_seq: each sub-stream should have a sequence for packet loss detection 
 * @loss_pack_cnt: counting loss packets
 * @recv_pack_cnt: counting recvived packets
 * @loss_rate: is equal to loss_pack_cnt/sub_seq
 * **/
struct mlo_recv_separate_stream{
    pack_head* pack_queue;
    struct sockaddr_in* target_addr;
    struct sockaddr_in* local_addr;
    pthread_t recv_tid;
    int sockfd;
    int sub_seq;
    int loss_pack_cnt;
    int recv_pack_cnt;
    double loss_rate;
};

/***
 * @brief record stream infomation
 * @sub_stream: store sub-stream infomation. use malloc to get memory.
 * @merged_head:
 * 
 * **/
struct mlo_recv_merged_stream{
    struct mlo_recv_separate_stream* sub_stream;
    pack_head* merged_head;
    pthread_t write_tid;
    pthread_t order_tid;
    FILE* write_file;
    int seq;
    int stream_cnt;
    int total_loss;
    int total_recvived;
};

typedef struct mlo_recv_separate_stream recv_unit;
typedef struct mlo_recv_merged_stream write_stream;

write_stream* stream_init(cJSON* json);

#ifdef __cplusplus
}
#endif