#include "lg/trade_record.h"
uint32_t lg_crc32(const void *ptr,uint32_t n){const uint8_t*p=ptr;uint32_t c=0xffffffffu;while(n--){c^=*p++;for(int k=0;k<8;k++)c=(c>>1)^(0xedb88320u&-(int32_t)(c&1));}return ~c;}
int lg_trade_record_valid(const LGTradeRecord*r){if(!r||r->magic!=LGTR_MAGIC||r->version!=LGTR_VERSION||r->state>LGTR_CANCELLED||r->payload_len>LGTR_PAYLOAD_MAX)return 0;return lg_crc32(r->payload,r->payload_len)==r->payload_crc32;}
