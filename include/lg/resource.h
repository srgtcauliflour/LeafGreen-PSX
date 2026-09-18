#pragma once
#include <stddef.h>
#include <stdint.h>
typedef enum { LG_RES_NONE=0,LG_RES_TEXTURE,LG_RES_MAP,LG_RES_SCRIPT,LG_RES_AUDIO } LGResourceKind;
typedef struct { uint32_t id; LGResourceKind kind; uint32_t offset; uint32_t size; } LGResourceEntry;
typedef struct { const LGResourceEntry *entries; size_t count; } LGResourceIndex;
const LGResourceEntry *lg_resource_find(const LGResourceIndex *index,uint32_t id);
/* Resolves id to a byte span within buffer (buffer_size bytes), the
   caller's already-loaded backing data for whatever "area bundle" the
   index describes (see RESOURCE-MANAGER.md) -- this does no I/O of its
   own, CD-scheduled or otherwise. Returns NULL and leaves *out_size
   untouched if id isn't found, its kind doesn't match expected_kind, or
   its offset+size doesn't fit entirely within buffer_size (an
   overflowing or malformed entry is never trusted, not even partially).
   On success, returns buffer+entry->offset and sets *out_size to
   entry->size. */
const uint8_t *lg_resource_bytes(const LGResourceIndex *index, uint32_t id,
                                  LGResourceKind expected_kind,
                                  const uint8_t *buffer, size_t buffer_size,
                                  size_t *out_size);
