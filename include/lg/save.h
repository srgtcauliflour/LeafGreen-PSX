#pragma once
#include <stddef.h>
#include <stdint.h>
#define LG_SAVE_MAGIC 0x5653474cU /* LGSV */
#define LG_SAVE_VERSION 2
typedef struct { uint32_t magic; uint16_t version; uint16_t flags; uint64_t save_id; uint32_t payload_size; uint32_t payload_crc32; } LGSaveHeader;
uint32_t lg_save_crc32(const void *data,size_t len);
int lg_save_header_valid(const LGSaveHeader *h,const void *payload,size_t available);
