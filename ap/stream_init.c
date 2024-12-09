#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <errno.h>
#include "mlo_pack.h"
#include "stream_init.h"
#include "json_wrapper.h"
#include "link_monitor.h"
#include "mlo_time.h"

#define SOCK_BUFFER_SIZE    (1024*1024*2)
#define READ_TIMEOUT_US     (1000*1000)
#define READ_PACKS          (1000)
static mlo_time_t mlo_timeout = {10, 0};

int init_sockaddr(struct sockaddr_in* sock_addr, char* ip, uint16_t port){
    memset(sock_addr, 0, sizeof(struct sockaddr_in));

    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port = htons(port);

    inet_pton(AF_INET, ip, &sock_addr->sin_addr);
    return 0;
}

int creat_ap_sock(struct sockaddr_in* sock_addr, char* ip, int bufsize, uint16_t port){
    init_sockaddr(sock_addr, ip, port);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    int status = bind(sock_fd, (struct sockaddr*) sock_addr, sizeof(struct sockaddr));

    if(status != 0){
        perror("error occurs in bind function");
    }

    return sock_fd;
}

recv_unit* init_recv_unit(recv_unit* unit_recv, char* local_ip, char* target_ip, uint16_t port){
    unit_recv->pack_queue = init_pack_queue();
    unit_recv->target_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    unit_recv->local_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    unit_recv->sockfd = creat_ap_sock(unit_recv->local_addr, local_ip, SOCK_BUFFER_SIZE, port);
    unit_recv->recv_pack_cnt = 0;

    init_sockaddr(unit_recv->target_addr, target_ip, port);
    connect(unit_recv->sockfd, (struct sockaddr*)(unit_recv->target_addr), sizeof(struct sockaddr_in));

    unit_recv->loss_pack_cnt = 0;
    unit_recv->sub_seq = -1;
    return unit_recv;
}

uint8_t unit_all_empty(int interface_num, recv_unit* unit_recv){
    for(int i = 0; i < interface_num; ++i){
        if(unit_recv[i].pack_queue == NULL) {
            fprintf(stderr, "recv unit get null pack queue in unit_all_empty function\n");
        }
        else if(!pack_queue_empty(unit_recv[i].pack_queue)) return 0;
    }
    return 1;
}

write_stream* stream_init(cJSON* json){
    write_stream* stream = (write_stream*)malloc(sizeof(write_stream));
    stream->merged_head = init_pack_queue();
    if(stream->merged_head == NULL) perror("error malloc merge head");

    int interface_num = get_if_cnt(json);
    uint16_t port = get_transfer_port(json);

    stream->sub_stream = (recv_unit*)malloc(sizeof(recv_unit)*interface_num);
    for(int i = 0; i < interface_num; ++i){
        char* local_ip = get_local_ip(json, i), *remote_ip = get_remote_ip(json, i);
        init_recv_unit(&stream->sub_stream[i], local_ip, remote_ip, port);
    }
    char* file_path = get_dest_file(json);

    stream->write_file = fopen(file_path, "w+");

    stream->stream_cnt = interface_num;
    stream->seq = -1;
    return stream;
}

