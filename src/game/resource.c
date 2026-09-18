#include "lg/resource.h"
const LGResourceEntry *lg_resource_find(const LGResourceIndex*i,uint32_t id){if(!i||!i->entries)return 0;for(size_t n=0;n<i->count;n++)if(i->entries[n].id==id)return &i->entries[n];return 0;}
const uint8_t *lg_resource_bytes(const LGResourceIndex *index,uint32_t id,LGResourceKind expected_kind,const uint8_t *buffer,size_t buffer_size,size_t *out_size){
 if(!buffer||!out_size) return 0;
 const LGResourceEntry *e=lg_resource_find(index,id);
 if(!e||e->kind!=expected_kind) return 0;
 size_t offset=e->offset,size=e->size;
 if(offset>buffer_size||size>buffer_size-offset) return 0;
 *out_size=size;
 return buffer+offset;
}
