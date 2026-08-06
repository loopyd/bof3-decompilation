#ifndef EMI_GAME_01_INTERNAL_H
#define EMI_GAME_01_INTERNAL_H

#include "bof3/bof3.h"
#include "frontend/state.h"
#include "frontend/selection.h"

typedef void (*GameFrontStateHandler)(void);

extern volatile u16 GAME_FRONT_EFFECT_BUSY;
extern volatile u16 GAME_FRONT_PAD_STATE;
extern volatile u16 GAME_FRONT_STATE;
extern volatile u16 GAME_FRONT_SUBSTATE;
extern volatile u16 GAME_FRONT_TIMER;
extern volatile u16 GAME_FRONT_BANNER_SCROLL;
extern volatile u16 GAME_FRONT_BANNER_ALPHA;
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_PRIMARY;
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_SECONDARY;
/* Low-byte u8 views of the two u16 alpha globals at the same address. The
 * draw calls read a single symbol-relative `lui/lbu` of the low byte; reading
 * `(u8)` of the volatile u16 emits `lhu+andi` instead. These raw data aliases
 * resolve by their hex suffix (no map entry: the address-keyed map allows only
 * one name per address), and are bound in symbols.c for the full build. */
extern volatile u8  D_80143C26;
extern volatile u8  D_80143C28;
extern volatile u8  GAME_FRONT_FADE_PHASE;
extern volatile u8  GAME_FRONT_WINDOW_PHASE;
extern volatile u8  GAME_FRONT_INPUT_GATE;
extern u8           GAME_FRONT_SELECTION;
extern u8           GAME_FRONT_PALETTE_STAGE_SERIAL;
extern volatile u16 D_80143B40;
extern volatile u16 D_80143F20;
extern volatile u8  D_80144FC0;
extern volatile u8  D_80144FC1;
extern volatile u8  D_80144FC2;
extern volatile u8  D_80144FC3;
extern volatile u8  D_80145024;
extern volatile u8 D_80146874;
extern volatile u8 D_8014832E;
extern volatile u16 D_80143B90;
extern volatile u8  D_80143BB0;
#define D_80143BB0 g_GameState
extern volatile u8           D_80143C30;
extern volatile u32          GAME_FRONT_POPUP_WORD;
extern volatile u32          D_8014598C;
extern volatile u16          D_80143C2A;
/* @kind: table — GAME_FRONT_STATE-dispatched handler pointers, one per
 * frontend state (payload 0x104c holds eight 0x801D**** entries). */
extern GameFrontStateHandler game_front_state_handlerTable[];
/* One 10-byte glyph geometry record; indexed per glyph id. */
typedef struct GameFrontGlyphGeometry {
  u16 unk_0;
  u16 unk_2;
  u16 unk_4;
  u16 unk_6;
  u16 unk_8;
} GameFrontGlyphGeometry;
/* @kind: table — glyph geometry records indexed by glyph id. */
extern GameFrontGlyphGeometry game_front_glyph_geometryTable[];

void func_8014BA04(void);
void game_front_update_banner(void);
void game_front_update_windows(void);
void func_8019611C(void);
void func_801A7704(u8 scenario_index);
void func_80197068(void);
int  func_8017B2B4(void);
void func_8017C2D8(u32 object, s32 x, s32 y, s32 flags, s32 arg4);
void func_8017AA1C(u8* primitive);
void func_8017A904(u8* primitive, u8 flags);
void func_8014E5A0(s32 group, s32 id);
u8*  game_front_draw_glyph(s32 x, s32 y, s32 width, s32 height, u8 flags);
void game_front_set_glyph_alpha(u8* primitive, u8 alpha);

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

void game_front_title_setup(void);
void game_front_main_loop(void);
void game_front_arm_fade_delay(void);
void game_front_tick_fade_timer(void);
void game_front_finish_selection(void);
void game_front_handle_menu_input(void);
void game_front_update_prompt(void);
void game_front_draw_prompt(void);
void game_front_draw_prompt_panels(u8 selected, u8 alpha);
void game_front_draw_label_groups(s16 x, s16 y, u8 selected, u8 alpha);
void game_front_draw_label_group(s16 x, s16 y, u8 selected, u8 alpha);
void game_front_open_selection(void);
void game_front_start_selection_fx(void);
void game_front_stop_selection_fx(void);
void game_front_finalize_exit(void);
void game_front_pre_dispatch_gate(void);

#define GAME_FRONT_START_MASK         0x0800u
#define GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define GAME_FRONT_SELECTION_FX_TABLE PSX_PTR(const volatile u8, 0x80181ebau)
#define GAME_FRONT_STATE_HANDLERS     game_front_state_handlerTable

#endif
