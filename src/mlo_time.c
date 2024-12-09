#include "mlo_time.h"
#include <stddef.h>

int mlo_compare_time(mlo_time_t* time1, mlo_time_t* time2){
    if(time1->tv_sec > time2->tv_sec)
        return 1;
    if(time1->tv_sec < time2->tv_sec)
        return -1;
    if(time1->tv_usec > time2->tv_usec)
        return 1;
    if(time1->tv_usec < time2->tv_usec)
        return -1;
    return 0;
}

int mlo_diff_time(mlo_time_t* time1, mlo_time_t* time2, mlo_time_t* dest){
    if(mlo_compare_time(time1, time2) == -1)
        return mlo_diff_time(time2, time1, dest);
    dest->tv_sec = time1->tv_sec-time2->tv_sec;
    dest->tv_usec = time1->tv_usec;
    if(dest->tv_usec < time2->tv_usec){
        dest->tv_sec -= 1;
        dest->tv_usec += 1000000;
    }
    dest->tv_usec -= time2->tv_usec;
    return 0;
}

int mlo_add_time(mlo_time_t* time1, mlo_time_t* time2, mlo_time_t* dest){
    dest->tv_sec = time1->tv_sec+time2->tv_sec;
    dest->tv_usec = time1->tv_usec+time2->tv_usec;
    if(dest->tv_usec > 1000000){
        dest->tv_usec -= 1000000;
        dest->tv_sec += 1;
    }
    return 0;
}

uint64_t mlo_time2us(mlo_time_t* time1){
    return time1->tv_sec*1000000ul+time1->tv_usec;
}

double mlo_time2sec(mlo_time_t* time1){
    return time1->tv_sec+time1->tv_usec/1000000.0;
}

int mlo_time_now(mlo_time_t* dest){
    return gettimeofday(dest, NULL);
}

int mlo_time_add_us(mlo_time_t* time1, uint64_t us){
    time1->tv_usec += us;
    time1->tv_sec += time1->tv_usec/1000000;
    time1->tv_usec = time1->tv_usec%1000000; 
}

int mlo_time_add_s(mlo_time_t* time1, uint32_t s){
    time1->tv_sec += s;
}