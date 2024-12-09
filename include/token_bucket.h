#pragma once

#include <unistd.h>
#include <stdint.h>

#define TOKEN_BUCKET_SIZE 2000

#ifdef __cplusplus
extern "C" {
#endif

struct mlo_token_bucket{
    uint64_t last_time;
    uint32_t token_left;
    double unit_time;
    uint32_t unit_cnt;
};

typedef struct mlo_token_bucket mlo_token_bucket_t;

int supply_token(mlo_token_bucket_t* bucket);

//block
void get_token(mlo_token_bucket_t* bucket);

//noblock
int try_gettoken(mlo_token_bucket_t* bucket);
int token_bucket_init(mlo_token_bucket_t* bucket, double rate);

#ifdef __cplusplus
}
#endif