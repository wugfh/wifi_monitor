#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int macaddr_aton(uint8_t* mac_addr, char* src);
int macaddr_ntoa(char* dest, uint8_t* mac_addr);
int enable_interface(char* if_name);
uint8_t char2hex(char tmp1);

struct tok{
    uint32_t v;
    const char* s;
};

#ifdef __cplusplus
}
#endif