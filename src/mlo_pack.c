#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mlo_pack.h"


mlo_pack* init_pack(int seq, const uint8_t* data, int len){
    mlo_pack* pack = (mlo_pack*)malloc(sizeof(mlo_pack));
    if(pack != NULL){
        if(len > 0){
            pack->payload = (uint8_t*)malloc(len);
            if(pack->payload == NULL) {
                free(pack);
                fprintf(stderr, "failed to malloc pack payload\n");
                return NULL;
            }
            memcpy(pack->payload, data, len);
        }
        else {
            pack->payload = NULL;
        }
        pack->payload_len = len;
        pack->seq = seq;
    }
    else {
        fprintf(stderr, "failed to malloc pack\n");
    }
    return pack;
}


mlo_pack* init_pack_fromfile(int seq, FILE* fp){
    if(feof(fp)) return NULL;
    mlo_pack* pack = (mlo_pack*)malloc(sizeof(mlo_pack));
    bzero(pack, sizeof(mlo_pack));
    if(pack != NULL){
        pack->payload = (uint8_t*)malloc(PACKET_PAYLOAD_LEN);
        if(pack->payload == NULL){
            free(pack);
            return NULL;
        }
        pack->payload_len = fread(pack->payload, sizeof(uint8_t), PACKET_PAYLOAD_LEN, fp);
        pack->seq = seq;
        if(pack->payload_len != PACKET_PAYLOAD_LEN){
            if(ferror(fp)){
                perror("读取文件错误");
                destory_pack(pack);
                return NULL;
            }
        }
    }
    return pack;
}


int destory_pack(mlo_pack* pack){
    if(pack == NULL) return -1;
    if(pack->payload != NULL)
        free(pack->payload);
    free(pack);
    return 1;
}


pack_head* init_pack_queue(){
    pack_head* queuehead = (pack_head*)malloc(sizeof(pack_head));
    if(queuehead != NULL){
        pthread_mutex_init(&queuehead->queue_lock, NULL);
        TAILQ_INIT(&queuehead->head);
        queuehead->pack_cnt = 0;
    }
    return queuehead;
}


int destory_pack_queue(pack_head* queue_head){
    mlo_pack* first_pack;
    pthread_mutex_lock(&queue_head->queue_lock);
    while(!TAILQ_EMPTY(&queue_head->head)){
        first_pack = TAILQ_FIRST(&queue_head->head);
        TAILQ_REMOVE(&queue_head->head, first_pack, pack_queue);
        destory_pack(first_pack);
    }
    pthread_mutex_unlock(&queue_head->queue_lock);
    pthread_mutex_destroy(&queue_head->queue_lock);
    free(queue_head);
    return 1;
}


mlo_pack* insert_pack_tail(pack_head* queue_head, mlo_pack* pack){
    pthread_mutex_lock(&queue_head->queue_lock);
    TAILQ_INSERT_TAIL(&queue_head->head, pack, pack_queue);
    queue_head->pack_cnt += 1;
    pthread_mutex_unlock(&queue_head->queue_lock);
    return pack;
}


int pack_queue_empty(pack_head* queue_head){
    pthread_mutex_lock(&queue_head->queue_lock);
    int status = TAILQ_EMPTY(&queue_head->head);
    pthread_mutex_unlock(&queue_head->queue_lock);
    return status;
}


mlo_pack* remove_pack_first(pack_head* queue_head){
    pthread_mutex_lock(&queue_head->queue_lock);
    mlo_pack* pack = TAILQ_FIRST(&queue_head->head);
    if(pack == NULL) return NULL;
    TAILQ_REMOVE(&queue_head->head, pack, pack_queue);
    queue_head->pack_cnt -= 1;
    pthread_mutex_unlock(&queue_head->queue_lock);
    return pack;
}


int delete_pack_first(pack_head* queue_head){
    mlo_pack* pack = remove_pack_first(queue_head);
    if(pack == NULL) return -1;
    destory_pack(pack);
    return 1;
}

mlo_pack* pack_queue_first(pack_head* queue_head){
    pthread_mutex_lock(&queue_head->queue_lock);
    mlo_pack* pack = TAILQ_FIRST(&queue_head->head);
    pthread_mutex_unlock(&queue_head->queue_lock);
    return pack;
}

mlo_pack* pack_queue_last(pack_head* head){
    pthread_mutex_lock(&head->queue_lock);
    mlo_pack* pack = TAILQ_LAST(&head->head, queue_head);
    pthread_mutex_unlock(&head->queue_lock);
    return pack;
}

/**
 * @brief insert pack in order
 * @return the pack be inserted 
 * 
 * **/
mlo_pack* insert_pack_order(int seq, pack_head* head, mlo_pack* target_pack){
    pthread_mutex_lock(&head->queue_lock);
    mlo_pack* pack = TAILQ_LAST(&head->head, queue_head);
    if(TAILQ_EMPTY(&head->head) || pack->seq < seq){
        TAILQ_INSERT_TAIL(&head->head, target_pack, pack_queue);
    }
    else {
        TAILQ_FOREACH_REVERSE(pack, &head->head, queue_head, pack_queue){
            if(pack->seq > seq){
                TAILQ_INSERT_BEFORE(pack, target_pack, pack_queue);
                break;
            }
        }
    }
    // TAILQ_FOREACH(pack, &head->head, pack_queue){
    //     printf("%d ", pack->seq);
    // }
    // printf("\n");
    // fflush(stdout);
    head->pack_cnt += 1;
    pthread_mutex_unlock(&head->queue_lock);
    return target_pack;
}

/**
 * @brief write the pack content into file 
 * @return the count of byte be written 
 * 
 * **/
int write_pack_file(mlo_pack* pack, FILE* fp){
    if(pack == NULL) return -1;
    int n = fwrite(pack->payload, sizeof(pack->payload[0]), pack->payload_len, fp);
    fflush(fp);
    return n;
}

int pack_queue_cnt(pack_head* head){
    pthread_mutex_lock(&head->queue_lock);
    int cnt = head->pack_cnt;
    pthread_mutex_unlock(&head->queue_lock);
    return cnt;
}

int packet2buf(mlo_pack* pack, uint8_t* send_buf, int sub_seq){
    int seq_nl = htonl(pack->seq);
    int payload_len_nl = htonl(pack->payload_len);
    int subseq_nl = htonl(sub_seq);
    int usec = pack->tstamp.tv_usec;
    int sec = pack->tstamp.tv_sec;
    usec = htonl(usec);
    sec = htonl(sec);
    memcpy(send_buf+SEQ_OFFSET, &seq_nl, sizeof(seq_nl));
    memcpy(send_buf+SUBSEQ_OFFSET, &subseq_nl, sizeof(seq_nl));
    memcpy(send_buf+USEC_OFFSET, &usec, sizeof(usec));
    memcpy(send_buf+SEC_OFFSET, &sec, sizeof(sec));
    memcpy(send_buf+PAYLOAD_LEN_OFFSET, &payload_len_nl, sizeof(payload_len_nl));
    memcpy(send_buf+PATLOAD_OFFSET, pack->payload, pack->payload_len);
    return pack->payload_len+PATLOAD_OFFSET;
} 