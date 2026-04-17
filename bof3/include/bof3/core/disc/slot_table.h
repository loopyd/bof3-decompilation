#ifndef BOF3_SLOT_TABLE_H
#define BOF3_SLOT_TABLE_H

#include "bof3/defines.h"

enum {
  BOF3_SLOT_DEMO_EMI = 0x25fu,
  BOF3_SLOT_FIRST_EMI = 0x261u,
  BOF3_SLOT_GAME_EMI = 0x262u,
  BOF3_SLOT_SCENA16_EMI = 0x2a5u,
  BOF3_SLOT_CAPCOM30_STR = 0x373u,
  BOF3_SLOT_LOGO_EXE = 0x374u,
};

typedef struct SlotTableEntry SlotTableEntry;

const SlotTableEntry* slot_table_find(u32 slot_id);
const SlotTableEntry* slot_table_logo_str(void);
const SlotTableEntry* func_8014aee0(void);

#endif
