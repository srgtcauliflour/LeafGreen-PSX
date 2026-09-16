#include "lg/resource.h"
const LGResourceEntry *lg_resource_find(const LGResourceIndex*i,uint32_t id){if(!i||!i->entries)return 0;for(size_t n=0;n<i->count;n++)if(i->entries[n].id==id)return &i->entries[n];return 0;}
