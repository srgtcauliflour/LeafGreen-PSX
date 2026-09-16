#include <assert.h>
#include <string.h>
#include "lg/save.h"
int main(void){unsigned char p[]={1,2,3};LGSaveHeader h;memset(&h,0,sizeof h);h.magic=LG_SAVE_MAGIC;h.version=LG_SAVE_VERSION;h.save_id=123;h.payload_size=3;h.payload_crc32=lg_save_crc32(p,3);assert(lg_save_header_valid(&h,p,3));p[0]^=1;assert(!lg_save_header_valid(&h,p,3));return 0;}
