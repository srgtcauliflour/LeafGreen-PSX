#pragma once
#include <stddef.h>
#include <stdint.h>
typedef enum { LG_RES_NONE=0,LG_RES_TEXTURE,LG_RES_MAP,LG_RES_SCRIPT,LG_RES_AUDIO } LGResourceKind;
typedef struct { uint32_t id; LGResourceKind kind; uint32_t offset; uint32_t size; } LGResourceEntry;
typedef struct { const LGResourceEntry *entries; size_t count; } LGResourceIndex;
const LGResourceEntry *lg_resource_find(const LGResourceIndex *index,uint32_t id);
