#ifndef LOADER_EMI_H
#define LOADER_EMI_H

#include "base/types.h"

typedef u32 EmiLoaderSlotId;
typedef u32 DiscLba;

typedef enum EmiLoaderSlot {
  EMI_LOADER_SLOT_DEMO_EMI = 0x25fu,
  EMI_LOADER_SLOT_FIRST_EMI = 0x261u,
  EMI_LOADER_SLOT_GAME_EMI = 0x262u,
  EMI_LOADER_SLOT_SCENA16_EMI = 0x2a5u,
  EMI_LOADER_SLOT_CAPCOM30_STR = 0x373u,
  EMI_LOADER_SLOT_LOGO_EXE = 0x374u,
} EmiLoaderSlot;

typedef struct SlotTableEntry SlotTableEntry;

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

typedef struct EmiActiveEntry {
  u32 active_lba;
  u32 remaining_size;
  u32 load_arg;
  u32 first_word;
  u16 type;
  bool header_mode;
} EmiActiveEntry;

typedef struct EmiLoaderEntry {
  u32 unknown_00;
  u32 source;
  u32 destination;
  u32 alternate_destination;
  s16 resource_id;
  u16 flags;
} EmiLoaderEntry;

#define LdrStreamInitSlot func_80161FDC
#define LdrIsReady        func_80162D00

#endif
