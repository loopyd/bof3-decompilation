#ifndef CORE_EMI_H
#define CORE_EMI_H

#include "bof3/defines.h"

enum {
  EMI_SECTOR_SIZE = 0x800,
  EMI_MAGIC_OFFSET = 0x08,
  EMI_MAGIC_SIZE = 8,
};

typedef struct EmiArchiveHeader {
  u32 entry_count;
  u32 version;
  u8  magic[EMI_MAGIC_SIZE];
} EmiArchiveHeader;

typedef struct EmiTocEntry {
  u32 size;
  u32 load_arg;
  u32 first_word;
  u16 type;
  u16 padding;
} EmiTocEntry;

void func_80161FDC(u32 slot_id);
s32  func_80162D00(void);

#define emi_stream_init_slot func_80161FDC
#define emi_loader_is_ready  func_80162D00

#endif
