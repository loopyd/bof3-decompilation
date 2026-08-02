#ifndef BOF3_CORE_H
#define BOF3_CORE_H

#include "base/types.h"
#include "loader/emi.h"

/* ---- callback scheduler ---- */
void func_8014B73C(void);
void func_8014B854(s32 slot_index, void (*callback)(void));
void func_8014B87C(u16 countdown);
void func_8014B8B0(void);

const SlotTableEntry* slot_table_logo_str(void);

/* ---- EMI archive ---- */
void func_80161FDC(u32 slot_id);
s32  func_80162D00(void);


/* ---- game front ---- */
void func_8014ECAC(u16 local_mode);
void func_80158DB8(u8 menu_id, u8 mode);
void func_8015DF18(u16 cue_id);
void func_80161808(u32 layout_bank);
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80161CD0(u8 selection_id, s32 cue_level, s32 cue_shape);

#endif
