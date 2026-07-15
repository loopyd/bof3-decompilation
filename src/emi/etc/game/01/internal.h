#ifndef EMI_GAME_01_INTERNAL_H
#define EMI_GAME_01_INTERNAL_H

#include "bof3/bof3.h"

extern vu16 GAME_FRONT_EFFECT_BUSY;
extern vu16 GAME_FRONT_PAD_STATE;
extern vu16 GAME_FRONT_STATE;
extern vu16 GAME_FRONT_SUBSTATE;
extern vu16 GAME_FRONT_TIMER;
extern vu16 GAME_FRONT_BANNER_SCROLL;
extern vu16 GAME_FRONT_BANNER_ALPHA;
extern vu16 GAME_FRONT_WINDOW_ALPHA_PRIMARY;
extern vu16 GAME_FRONT_WINDOW_ALPHA_SECONDARY;
extern vu8  GAME_FRONT_FADE_PHASE;
extern vu8  GAME_FRONT_WINDOW_PHASE;
extern vu8  GAME_FRONT_INPUT_GATE;
extern u8   GAME_FRONT_SELECTION;
extern vu8  GAME_FRONT_PALETTE_STAGE_SERIAL;
extern vu16 D_80143B40;
extern vu16 D_80143F20;
extern vu8  D_80144FC0;
extern vu8  D_80144FC1;
extern vu8  D_80144FC2;
extern vu8  D_80144FC3;
extern vu8  D_80145024;
extern vu8  D_80146874;
extern vu8  D_8014832E;
extern vu16 D_80143B90;
extern vu8  D_80143BB0;
extern vu8  D_80143C30;
extern vu32 GAME_FRONT_POPUP_WORD __asm__("D_80143C30");
extern vu32 D_8014598C;
extern vu16 D_80143C2A;
#define GAME_FRONT_START_MASK         0x0800u
#define GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define GAME_FRONT_SELECTION_FX_TABLE CVPTR(u8, 0x80181ebau)

typedef void (*GameFrontStateHandler)(void);
extern GameFrontStateHandler D_801D1C4C[];
#define GAME_FRONT_STATE_HANDLERS D_801D1C4C

void func_8014ba04(void);
void func_801d18f8(void);
void func_801d1b00(void);
void func_8019611c(void);
void func_801a7704(u8 scenario_index);
void func_80197068(void);
int  func_8017b2b4(void);
void func_8017c2d8(u32 object, s32 x, s32 y, s32 flags, s32 arg4);
void func_8017aa1c(u8* primitive);
void func_8017a904(u8* primitive, u8 flags);
void func_8014e5a0(s32 group, s32 id);
u8*  func_801d17d8(s32 x, s32 y, s32 width, s32 height, u8 flags);
void func_801d18e8(u8* primitive, u8 alpha);

/* @behavior slot-2 frontend-local callback body selected by the local mode byte.
 * @source 0x8014ed6c
 */
void game_front_local_mode_callback_loop(void);

/* @behavior starts one selection-specific frontend effect by table id pair.
 * @source 0x8015d4f8
 */
void game_start_selection_fx(u32 effect_group, s32 effect_id, s32 duration,
                             s32 fade_step);

/* @behavior stops one selection-specific frontend effect by table id pair.
 * @source 0x8015d404
 */
void game_stop_selection_fx(u32 effect_group, s32 effect_id);

/* @behavior starts streaming one archive slot through the EXE-side EMI loader.
 * @source 0x80161fdc
 */

/* @behavior copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source 0x8014e284
 */
void game_stage_shared_palette_bank(void);

/* @behavior queues one frontend cue/event id through the EXE-side dispatcher.
 * @source 0x8015df18
 */
void game_queue_frontend_cue(u32 cue_id);

void func_801d0c90(void);
void func_801d0c04(void);
void func_801d0d5c(void);
void func_801d0d94(void);
void func_801d0e54(void);
void func_801d0f00(void);
void func_801d0fb8(void);
void func_801d11e4(void);
void func_801d12cc(u8 selected, u8 alpha);
void func_801d150c(s16 x, s16 y, u8 selected, u8 alpha);
void func_801d16dc(s16 x, s16 y, u8 selected, u8 alpha);
void func_801d0df0(void);
void func_801d1134(void);
void func_801d1184(void);
void func_801d1000(void);
void func_801d104c(void);

#endif
