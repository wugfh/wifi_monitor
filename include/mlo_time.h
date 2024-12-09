#pragma once

#include <sys/time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct timeval mlo_time_t;

int mlo_compare_time(mlo_time_t* time1, mlo_time_t* time2);
int mlo_diff_time(mlo_time_t* time1, mlo_time_t* time2, mlo_time_t* dest);
int mlo_add_time(mlo_time_t* time1, mlo_time_t* time2, mlo_time_t* dest);
int mlo_time_add_us(mlo_time_t* time1, uint64_t us);
int mlo_time_add_s(mlo_time_t* time1, uint32_t s);
uint64_t mlo_time2us(mlo_time_t* time1);
double mlo_time2sec(mlo_time_t* time1);
int mlo_time_now(mlo_time_t* dest);

#ifdef __cplusplus
}
#endif