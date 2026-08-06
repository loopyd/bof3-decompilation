#ifndef EMI_WORLD00_AREA026_13_INTERNAL_H
#define EMI_WORLD00_AREA026_13_INTERNAL_H

#include "bof3/bof3.h"

extern volatile u32 D_8014686C;
/* @kind: bss — area counter; stepped by +/-0x14 via the counter
 * advance/retreat pair and cleared by counter_reset. */
extern volatile u16 world00_area026_counter;
extern volatile u8  D_80149333;

void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void func_8015B580(void* arg0, u8 bit_index);
void func_8015B5A8(void* arg0, u8 bit_index);
void func_801AFE18(void* arg0);
void func_80196070(void);
void func_801AFF04(const void* arg0, void* arg1);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_801F2D5C(const s32* arg0, s32 arg1);
void func_801F2E04(const s32* arg0, s32 arg1, s16 arg2, s32 arg3, s32 arg4);
void world00_area026_scenario_flags_set29_clear28(void);
void world00_area026_scenario_flags_clear29(void);
void world00_area026_scenario_flags_set28(void);
void world00_area026_scenario_flags_clear28(void);

#define WORLD00_AREA026_13_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA026_13_TABLE_33FC    PSX_PTR(const s32, 0x801f33fcu)
#define WORLD00_AREA026_13_TABLE_340C    PSX_PTR(const s32, 0x801f340cu)

#endif
