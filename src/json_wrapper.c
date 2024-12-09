#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include "util.h"
#include "json_wrapper.h"

cJSON* parse_json(char* json_file_path){
    FILE* fp = fopen(json_file_path, "r");
    struct stat file_stat;
    stat(json_file_path, &file_stat);
    int file_len = file_stat.st_size;
    char* buf = (char*)malloc(file_len+10);
    fread(buf, sizeof(char), file_len, fp);
    cJSON* json = cJSON_Parse(buf);
    free(buf);
    return json;
}

/**
 * @return 0:AP   1:STA     -1:error
 * 
 * 
 * **/
int get_name(cJSON* json){
    cJSON* mode = cJSON_GetObjectItem(json, "name");
    char* mode_value = cJSON_GetStringValue(mode);
    if(strcmp(mode_value, "STA") == 0)
        return RUN_IN_STA;
    else if(strcmp(mode_value, "AP") == 0)
        return RUN_IN_AP;
    else{
        fprintf(stderr, "name must be AP or STA\n");
    }
}

/**
 * @return 0:throughput    1:latency    -1:error
 * 
 * 
 * **/
int get_mode(cJSON* json){
    cJSON* mode = cJSON_GetObjectItem(json, "mode");
    char* mode_value = cJSON_GetStringValue(mode);
    if(strcmp(mode_value, "throughput") == 0)
        return 0;
    else if(strcmp(mode_value, "latency") == 0)
        return 1;
    else return -1;
}

char* get_src_file(cJSON* json){
    cJSON* src_path = cJSON_GetObjectItem(json, "source");
    return cJSON_GetStringValue(src_path);
}

char* get_dest_file(cJSON* json){
    cJSON* dest_path = cJSON_GetObjectItem(json, "destination");
    return cJSON_GetStringValue(dest_path);
}

char* get_local_ip(cJSON* json, int index){
    cJSON* local_array = cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);
    cJSON* local_ip = cJSON_GetObjectItem(local_if, "IP");
    return cJSON_GetStringValue(local_ip);
}

char* get_remote_ip(cJSON* json, int index){
    cJSON* remote_array = cJSON_GetObjectItem(json, "remote");
    cJSON* remote_if = cJSON_GetArrayItem(remote_array, index);
    cJSON* remote_ip = cJSON_GetObjectItem(remote_if, "IP");
    return cJSON_GetStringValue(remote_ip);
}

char* get_local_interface(cJSON* json, int index){
    cJSON* local_array = cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);
    cJSON* local_interface = cJSON_GetObjectItem(local_if, "interface");
    return cJSON_GetStringValue(local_interface);
}

int get_if_cnt(cJSON* json){
    cJSON* remote_array = cJSON_GetObjectItem(json, "local");
    return cJSON_GetArraySize(remote_array);
}

double get_if_bitrate(cJSON* json, int index){
    cJSON* local_array =cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);
    cJSON* bitrate_json = cJSON_GetObjectItem(local_if, "bitrate");
    char bitrate_string[10];
    strcpy(bitrate_string, cJSON_GetStringValue(bitrate_json));
    int len = strlen(bitrate_string);
    char magnitude = bitrate_string[len-1];
    if(magnitude >= '0' && magnitude <= '9'){
        magnitude = '0';
    }
    else {
        bitrate_string[len-1] = '\0';
        if(magnitude >= 'a' && magnitude <= 'z')
            magnitude = magnitude-'a'+'A';
    }
    double num = atoi(bitrate_string);
    switch (magnitude)
    {
    case 'G':
        num *= 1000;
    case 'M':
        num *= 1000;
    case 'K':
        num *= 1000;
        break;
    default:
        break;
    }
    return num/1000000;
}

uint16_t get_transfer_port(cJSON* json){
    cJSON* port_json = cJSON_GetObjectItem(json, "port");
    uint16_t ret = cJSON_GetNumberValue(port_json);
    return ret;
}

double get_bernoulli_param(cJSON* json){
    cJSON* port_json = cJSON_GetObjectItem(json, "bernoulli_param");
    double ret = cJSON_GetNumberValue(port_json);
    return ret;
}

double get_interface_probability(cJSON* json){
    cJSON* port_json = cJSON_GetObjectItem(json, "interface_probability");
    double ret = cJSON_GetNumberValue(port_json);
    return ret;
}

uint32_t get_pack_cnt(cJSON* json){
    cJSON* port_json = cJSON_GetObjectItem(json, "packs_number");
    uint32_t ret = cJSON_GetNumberValue(port_json);
    return ret;
}

uint32_t get_time_slot(cJSON* json){
    cJSON* port_json = cJSON_GetObjectItem(json, "time_slot");
    uint32_t ret = cJSON_GetNumberValue(port_json);
    return ret;
}

int get_mcs_status(mcs_status_t* mcs, cJSON* json, int index){
    cJSON* local_array = cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);
    cJSON* obj = cJSON_GetObjectItem(local_if, "mcs_family");
    char* family = cJSON_GetStringValue(obj);
    if(strcmp(family, "HT") == 0){
        mcs->family = HT;
    }
    else if(strcmp(family, "VHT") == 0){
        mcs->family = VHT;
    }
    else if(strcmp(family, "HE") == 0){
        mcs->family = HE;
    }
    else if(strcmp(family, "LEGACY") == 0){
        mcs->family = LEGACY;
    }
    
    obj = cJSON_GetObjectItem(local_if, "frequency");
    int freq = cJSON_GetNumberValue(obj);
    if(freq == 5){
        mcs->freq = _5G;
    }
    else if(freq == 2){
        mcs->freq = _2_4G;
    }
    else if(freq == 6){
        mcs->freq = _6G;
    }

    obj = cJSON_GetObjectItem(local_if, "mcs");
    int mcs_index = cJSON_GetNumberValue(obj);
    if(mcs->family == LEGACY){
        mcs->legacy_rate = mcs_index;
    }
    else if(mcs->family == HT){
        mcs->mcs_index = mcs_index;
    }
    else if(mcs->family == VHT){
        cJSON* nss_obj = cJSON_GetObjectItem(local_if, "nss");
        mcs->nss = cJSON_GetNumberValue(nss_obj);
        mcs->mcs_index = mcs_index;
    }
    else if(mcs->family == HE){
        cJSON* nss_obj = cJSON_GetObjectItem(local_if, "nss");
        mcs->nss = cJSON_GetNumberValue(nss_obj);
        mcs->mcs_index = mcs_index;
    }
    return 0;
}

int get_monitor_ifstatus(interface_status_t* if_status, cJSON* json, int index){
    cJSON* local_array = cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);  
    cJSON* obj = cJSON_GetObjectItem(local_if, "monitor_name");
    char* mon_name = cJSON_GetStringValue(obj);
    strcpy(if_status->name, mon_name);
    obj = cJSON_GetObjectItem(local_if, "monitor_mac");
    char* mac_addr = cJSON_GetStringValue(obj);
    if(strlen(mac_addr) < ETH_ALEN){
        if_status->have_mac = 0;
    }
    else if(macaddr_aton(if_status->mac_addr, mac_addr) != 0){
        if_status->have_mac = 0;
        fprintf(stderr, "monitor mac addr is not valid, format:\"a:b:c:d:e:f\"\n");
    }
    else{
        if_status->have_mac = 1;
    }
    if_status->type = MONITOR;
    return 0;
}

int get_monitor_filter(cJSON* json, int index, char* filter_buf){
    cJSON* local_array = cJSON_GetObjectItem(json, "local");
    cJSON* local_if = cJSON_GetArrayItem(local_array, index);  
    cJSON* obj = cJSON_GetObjectItem(local_if, "filter");
    char* filter = cJSON_GetStringValue(obj);
    strcpy(filter_buf, filter);
    return 0;
}