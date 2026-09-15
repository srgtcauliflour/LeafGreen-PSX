#include "lg/save.h"
uint32_t lg_save_crc32(const void *ptr,size_t n){const uint8_t*p=ptr;uint32_t c=0xffffffffu;while(n--){c^=*p++;for(int k=0;k<8;k++)c=(c>>1)^(0xedb88320u&-(int32_t)(c&1));}return ~c;}
int lg_save_header_valid(const LGSaveHeader*h,const void*p,size_t n){if(!h||h->magic!=LG_SAVE_MAGIC||h->version!=LG_SAVE_VERSION||h->payload_size>n)return 0;if(h->payload_size&&!p)return 0;return lg_save_crc32(p,h->payload_size)==h->payload_crc32;}
