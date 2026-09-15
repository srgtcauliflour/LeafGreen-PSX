#include <assert.h>
#include <string.h>
#include "lg/trade_record.h"
int main(void){LGTradeRecord r;memset(&r,0,sizeof r);r.magic=LGTR_MAGIC;r.version=LGTR_VERSION;r.state=LGTR_OFFERED;r.payload[0]=42;r.payload_len=1;r.payload_crc32=lg_crc32(r.payload,1);assert(lg_trade_record_valid(&r));r.payload[0]=43;assert(!lg_trade_record_valid(&r));return 0;}
