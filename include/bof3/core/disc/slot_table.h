#ifndef SLOT_TABLE_H
#define SLOT_TABLE_H

#include "bof3/defines.h"

enum {
  SLOT_DEMO_EMI = 0x25fu,
  SLOT_FIRST_EMI = 0x261u,
  SLOT_GAME_EMI = 0x262u,
  SLOT_SCENA16_EMI = 0x2a5u,
  SLOT_CAPCOM30_STR = 0x373u,
  SLOT_LOGO_EXE = 0x374u,
};

typedef struct SlotTableEntry SlotTableEntry;

const SlotTableEntry* slot_table_find(u32 slot_id);
const SlotTableEntry* slot_table_logo_str(void);
const SlotTableEntry* func_8014aee0(void);

#endif
