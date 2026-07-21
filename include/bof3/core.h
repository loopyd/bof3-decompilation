#ifndef BOF3_CORE_H
#define BOF3_CORE_H

#include "bof3/defines.h"

/* ---- callback scheduler ---- */

void func_8014B73C(void);
void func_8014B854(s32 slot_index, void (*callback)(void));
void func_8014B87C(u16 countdown);
void func_8014B8B0(void);

/* ---- disc / slot table ---- */

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

const SlotTableEntry* slot_table_find(EmiLoaderSlotId slot_id);
const SlotTableEntry* slot_table_logo_str(void);

/* ---- EMI archive ---- */

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

/* ---- game front ---- */

void func_8014ECAC(u16 local_mode);
void func_80161808(u32 layout_bank);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80161CD0(u8 selection_id, s32 cue_level, s32 cue_shape);

#define game_set_frontend_layout_bank func_80161808
#define game_set_active_selection_cue func_80161C20

#endif
