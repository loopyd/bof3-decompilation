#ifndef EMI_WORLD00_AREA026_13_INTERNAL_H
#define EMI_WORLD00_AREA026_13_INTERNAL_H

#include "bof3/bof3.h"

/* @source 0x8014686C @kind unknown */
extern volatile u32 D_8014686C;
/* @source 0x8014932A @kind bss — area counter; stepped by +/-0x14 via the
 * counter advance/retreat pair and cleared by resetCounter. */
extern volatile u16 counter;
/* @source 0x80149333 @kind unknown */
extern volatile u8  D_80149333;

void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void func_8015B580(void* arg0, u8 bit_index);
void func_8015B5A8(void* arg0, u8 bit_index);

void func_801F2C48(void);

void setScenarioFlagBit29ClearBit28(void);
void clearScenarioFlagBit29(void);
void setScenarioFlagBit28(void);
void clearScenarioFlagBit28(void);

#endif
