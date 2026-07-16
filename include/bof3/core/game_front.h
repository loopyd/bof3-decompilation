#ifndef CORE_GAME_FRONT_H
#define CORE_GAME_FRONT_H

#include "bof3/defines.h"

void func_8014ECAC(u16 local_mode);

/* @behavior rebuilds the frontend layout-bank pointer set for the requested mode.
 * @source 0x80161808
 */
void func_80161808(u32 layout_bank);

/* @behavior starts the active selection cue/SEP and records the active selector.
 * @source 0x80161c20
 */
void func_80161C20(u8 selection_id, s32 cue_level, s32 cue_shape);

/* The semantic distinction from func_80161C20 is not yet proven. */
void func_80161CD0(u8 selection_id, s32 cue_level, s32 cue_shape);

/* Keep address-based linker symbols visible to matching and analysis tools. */
#define game_set_frontend_layout_bank func_80161808
#define game_set_active_selection_cue func_80161C20

#endif
