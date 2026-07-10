#ifndef BOF3_SRC_MODULES_GAME_01_INTERNAL_H
#define BOF3_SRC_MODULES_GAME_01_INTERNAL_H

#include "bof3/bof3.h"

extern vu16 GAME_FRONT_EFFECT_BUSY;
extern vu16 GAME_FRONT_PAD_STATE;
extern vu16 GAME_FRONT_STATE;
extern vu16 GAME_FRONT_TIMER;
extern vu16 GAME_FRONT_BANNER_SCROLL;
extern vu16 GAME_FRONT_BANNER_ALPHA;
extern vu16 GAME_FRONT_WINDOW_ALPHA_PRIMARY;
extern vu16 GAME_FRONT_WINDOW_ALPHA_SECONDARY;
extern vu8  GAME_FRONT_FADE_PHASE;
extern vu8  GAME_FRONT_WINDOW_PHASE;
extern vu8  GAME_FRONT_INPUT_GATE;
extern vu8  GAME_FRONT_SELECTION;
extern vu8  GAME_FRONT_PALETTE_STAGE_SERIAL;
#define GAME_FRONT_START_MASK         0x0800u
#define GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define GAME_FRONT_SELECTION_FX_TABLE CVPTR(u8, 0x80181ebau)

/* does: slot-2 frontend-local callback body selected by the local mode byte.
 * @source: 0x8014ed6c
 */
void game_front_local_mode_callback_loop(void);

/* does: rebuilds the frontend layout-bank pointer set for the requested mode.
 * @source: 0x80161808
 */
void game_set_frontend_layout_bank(u32 layout_bank);

/* does: starts one selection-specific frontend effect by table id pair.
 * @source: 0x8015d4f8
 */
void game_start_selection_fx(u32 effect_group, s32 effect_id, s32 duration,
                             s32 fade_step);

/* does: stops one selection-specific frontend effect by table id pair.
 * @source: 0x8015d404
 */
void game_stop_selection_fx(u32 effect_group, s32 effect_id);

/* does: starts streaming one archive slot through the EXE-side EMI loader.
 * @source: 0x80161fdc
 */
void emi_stream_init_slot(u32 slot_id);

/* does: starts the active selection cue/SEP and records the active selector.
 * @source: 0x80161c20
 */
void game_set_active_selection_cue(u8 selection_id, s32 cue_level,
                                   s32 cue_shape);

/* does: copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source: 0x8014e284
 */
void game_stage_shared_palette_bank(void);

/* does: queues one frontend cue/event id through the EXE-side dispatcher.
 * @source: 0x8015df18
 */
void game_queue_frontend_cue(u32 cue_id);

/* from old bof3/include/bof3/modules/game/01.h */
void func_801d0c90(void);
void func_801d0df0(void);
void func_801d1134(void);
void func_801d1184(void);
void func_801d1000(void);
void func_801d104c(void);

#endif
