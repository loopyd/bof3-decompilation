#ifndef EMI_GAME_01_INTERNAL_H
#define EMI_GAME_01_INTERNAL_H

#include "bof3/bof3.h"

extern volatile u16 GAME_FRONT_EFFECT_BUSY;
extern volatile u16 GAME_FRONT_PAD_STATE;
extern volatile u16 GAME_FRONT_STATE;
extern volatile u16 GAME_FRONT_SUBSTATE;
extern volatile u16 GAME_FRONT_TIMER;
extern volatile u16 GAME_FRONT_BANNER_SCROLL;
extern volatile u16 GAME_FRONT_BANNER_ALPHA;
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_PRIMARY;
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_SECONDARY;
extern volatile u8  GAME_FRONT_FADE_PHASE;
extern volatile u8  GAME_FRONT_WINDOW_PHASE;
extern volatile u8  GAME_FRONT_INPUT_GATE;
extern u8   GAME_FRONT_SELECTION;
extern volatile u8  GAME_FRONT_PALETTE_STAGE_SERIAL;
extern volatile u16 D_80143B40;
extern volatile u16 D_80143F20;
extern volatile u8  D_80144FC0;
extern volatile u8  D_80144FC1;
extern volatile u8  D_80144FC2;
extern volatile u8  D_80144FC3;
extern volatile u8  D_80145024;
extern volatile u8  D_80146874;
extern volatile u8  D_8014832E;
extern volatile u16 D_80143B90;
extern volatile u8  D_80143BB0;
extern volatile u8  D_80143C30;
extern volatile u32 GAME_FRONT_POPUP_WORD __asm__("D_80143C30");
extern volatile u32 D_8014598C;
extern volatile u16 D_80143C2A;
#define GAME_FRONT_START_MASK         0x0800u
#define GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define GAME_FRONT_SELECTION_FX_TABLE PSX_PTR(const volatile u8, 0x80181ebau)

typedef void (*GameFrontStateHandler)(void);
extern GameFrontStateHandler D_801D1C4C[];
#define GAME_FRONT_STATE_HANDLERS D_801D1C4C

void func_8014BA04(void);
void func_801D18F8(void);
void func_801D1B00(void);
void func_8019611C(void);
void func_801A7704(u8 scenario_index);
void func_80197068(void);
int  func_8017B2B4(void);
void func_8017C2D8(u32 object, s32 x, s32 y, s32 flags, s32 arg4);
void func_8017AA1C(u8* primitive);
void func_8017A904(u8* primitive, u8 flags);
void func_8014E5A0(s32 group, s32 id);
u8*  func_801D17D8(s32 x, s32 y, s32 width, s32 height, u8 flags);
void func_801D18E8(u8* primitive, u8 alpha);

/* @behavior slot-2 frontend-local callback body selected by the local mode byte.
 * @source 0x8014ED6C
 */
void game_front_local_mode_callback_loop(void);

/* @behavior starts one selection-specific frontend effect by table id pair.
 * @source 0x8015D4F8
 */
void game_start_selection_fx(u32 effect_group, s32 effect_id, s32 duration,
                             s32 fade_step);

/* @behavior stops one selection-specific frontend effect by table id pair.
 * @source 0x8015D404
 */
void game_stop_selection_fx(u32 effect_group, s32 effect_id);

/* @behavior starts streaming one archive slot through the EXE-side EMI loader.
 * @source 0x80161FDC
 */

/* @behavior copies the shared CPU-side palette bank before the corresponding VRAM
 * upload path.
 * @source 0x8014E284
 */
void game_stage_shared_palette_bank(void);

/* @behavior queues one frontend cue/event id through the EXE-side dispatcher.
 * @source 0x8015DF18
 */
void game_queue_frontend_cue(u32 cue_id);

void func_801D0C90(void);
void func_801D0C04(void);
void func_801D0D5C(void);
void func_801D0D94(void);
void func_801D0E54(void);
void func_801D0F00(void);
void func_801D0FB8(void);
void func_801D11E4(void);
void func_801D12CC(u8 selected, u8 alpha);
void func_801D150C(s16 x, s16 y, u8 selected, u8 alpha);
void func_801D16DC(s16 x, s16 y, u8 selected, u8 alpha);
void func_801D0DF0(void);
void func_801D1134(void);
void func_801D1184(void);
void func_801D1000(void);
void func_801D104C(void);

#endif
