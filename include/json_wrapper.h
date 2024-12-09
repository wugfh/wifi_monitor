#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

#include "cJSON.h"
#include "link_monitor.h"

enum{
    RUN_IN_STA,
    RUN_IN_AP,
};

cJSON* parse_json(char* json_file_path);
int get_name(cJSON* json);
int get_mode(cJSON* json);
char* get_src_file(cJSON* json);
char* get_dest_file(cJSON* json);
char* get_local_ip(cJSON* json, int index);
char* get_remote_ip(cJSON* json, int index);
char* get_local_interface(cJSON* json, int index);
int get_if_cnt(cJSON* json);
double get_if_bitrate(cJSON* json, int index);
uint16_t get_transfer_port(cJSON* json);
double get_bernoulli_param(cJSON* json);
double get_interface_probability(cJSON* json);
uint32_t get_pack_cnt(cJSON* json);
uint32_t get_time_slot(cJSON* json);
int get_mcs_status(mcs_status_t* mcs, cJSON* json, int index);
int get_monitor_ifstatus(interface_status_t* if_status, cJSON* json, int index);
int get_monitor_filter(cJSON* json, int index, char* filter_buf);

#ifdef __cplusplus
}
#endif