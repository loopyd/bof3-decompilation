#ifndef CORE_DISC_SLOT_TABLE_H
#define CORE_DISC_SLOT_TABLE_H

#include "bof3/defines.h"

/* Native loader input: an index into the u32 LBA table at D_80182444. */
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
/* Historical SLUS boot probe; it is not a native LOGO.EXE entry point. */
const SlotTableEntry* slot_table_logo_str(void);

#endif
