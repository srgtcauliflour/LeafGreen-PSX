#pragma once
#include <stdint.h>
#define LGTR_MAGIC 0x5254474cU /* 'LGTR' little-endian */
#define LGTR_VERSION 1
#define LGTR_PAYLOAD_MAX 512
typedef enum { LGTR_EMPTY=0,LGTR_OFFERED=1,LGTR_ACCEPTED=2,LGTR_COMPLETED=3,LGTR_CANCELLED=4 } LGTradeState;
typedef struct { uint32_t magic; uint16_t version; uint16_t state; uint64_t trade_id; uint64_t save_id; uint16_t payload_len; uint16_t reserved; uint32_t payload_crc32; uint8_t payload[LGTR_PAYLOAD_MAX]; } LGTradeRecord;
uint32_t lg_crc32(const void *data,uint32_t len);
int lg_trade_record_valid(const LGTradeRecord *r);
