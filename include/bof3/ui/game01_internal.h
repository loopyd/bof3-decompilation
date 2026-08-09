#ifndef EMI_GAME_01_INTERNAL_H
#define EMI_GAME_01_INTERNAL_H

#include "bof3/bof3.h"
#include "frontend/state.h"
#include "frontend/selection.h"

typedef void (*GameFrontStateHandler)(void);

/* @source 0x80143C40 @kind unknown */
extern volatile u16 GAME_FRONT_EFFECT_BUSY;
/* @source 0x80145AA8 @kind unknown */
extern volatile u16 GAME_FRONT_PAD_STATE;
/* @source 0x80143C10 @kind unknown */
extern volatile u16 GAME_FRONT_STATE;
/* @source 0x80143C12 @kind unknown */
extern volatile u16 GAME_FRONT_SUBSTATE;
/* @source 0x80143C20 @kind unknown */
extern volatile u16 GAME_FRONT_TIMER;
/* @source 0x80143C22 @kind unknown */
extern volatile u16 GAME_FRONT_BANNER_SCROLL;
/* @source 0x80143C24 @kind unknown */
extern volatile u16 GAME_FRONT_BANNER_ALPHA;
/* @source 0x80143C26 @kind unknown */
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_PRIMARY;
/* @source 0x80143C28 @kind unknown */
extern volatile u16 GAME_FRONT_WINDOW_ALPHA_SECONDARY;
/* Low-byte u8 views of the two u16 alpha globals at the same address. The
 * draw calls read a single symbol-relative `lui/lbu` of the low byte; reading
 * `(u8)` of the volatile u16 emits `lhu+andi` instead. These raw data aliases
 * resolve by their hex suffix (no map entry: the address-keyed map allows only
 * one name per address), and are bound in symbols.c for the full build. */
/* @source 0x80143C26 @kind unknown */
extern volatile u8  D_80143C26;
/* @source 0x80143C28 @kind unknown */
extern volatile u8  D_80143C28;
/* @source 0x80143C31 @kind unknown */
extern volatile u8  GAME_FRONT_FADE_PHASE;
/* @source 0x80143C32 @kind unknown */
extern volatile u8  GAME_FRONT_WINDOW_PHASE;
/* @source 0x80143C33 @kind unknown */
extern volatile u8  GAME_FRONT_INPUT_GATE;
/* @source 0x80145029 @kind unknown */
extern u8           GAME_FRONT_SELECTION;
/* @source 0x80145988 @kind unknown */
extern u8           GAME_FRONT_PALETTE_STAGE_SERIAL;
/* @source 0x80143B40 @kind unknown */
extern volatile u16 D_80143B40;
/* @source 0x80143F20 @kind unknown */
extern volatile u16 D_80143F20;
/* @source 0x80144FC0 @kind unknown */
extern volatile u8  D_80144FC0;
/* @source 0x80144FC1 @kind unknown */
extern volatile u8  D_80144FC1;
/* @source 0x80144FC2 @kind unknown */
extern volatile u8  D_80144FC2;
/* @source 0x80144FC3 @kind unknown */
extern volatile u8  D_80144FC3;
/* @source 0x80145024 @kind unknown */
extern volatile u8  D_80145024;
/* @source 0x80146874 @kind unknown */
extern volatile u8 D_80146874;
/* @source 0x8014832E @kind unknown */
extern volatile u8 D_8014832E;
/* @source 0x80143B90 @kind unknown */
extern volatile u16 D_80143B90;
/* @source 0x80143BB0 @kind unknown */
extern volatile u8  D_80143BB0;
#define D_80143BB0 g_GameState
/* @source 0x80143C30 @kind unknown */
extern volatile u8           D_80143C30;
/* @source 0x80143C30 @kind unknown */
extern volatile u32          GAME_FRONT_POPUP_WORD;
/* @source 0x8014598C @kind unknown */
extern volatile u32          D_8014598C;
/* @source 0x80143C2A @kind unknown */
extern volatile u16          D_80143C2A;
/* @source 0x801D1C4C @kind table — GAME_FRONT_STATE-dispatched handler
 * pointers, one per frontend state (payload 0x104c holds eight 0x801D****
 * entries). */
extern GameFrontStateHandler stateHandlerTable[];
/* One 10-byte glyph geometry record; indexed per glyph id. */
typedef struct GameFrontGlyphGeometry {
  u16 unk_0;
  u16 unk_2;
  u16 unk_4;
  u16 unk_6;
  u16 unk_8;
} GameFrontGlyphGeometry;
/* @source 0x801D1C6C @kind table — glyph geometry records indexed by glyph
 * id. */
extern GameFrontGlyphGeometry glyphGeometryTable[];

void func_8014BA04(void);
void updateBanner(void);
void updateWindows(void);
void func_8019611C(void);
void func_801A7704(u8 scenario_index);
void func_80197068(void);
int  func_8017B2B4(void);
void func_8017C2D8(u32 object, s32 x, s32 y, s32 flags, s32 arg4);
void func_8017AA1C(u8* primitive);
void func_8017A904(u8* primitive, u8 flags);
void func_8014E5A0(s32 group, s32 id);
u8*  drawGlyph(s32 x, s32 y, s32 width, s32 height, u8 flags);
void setGlyphAlpha(u8* primitive, u8 alpha);

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

void titleSetup(void);
void mainLoop(void);
void armFadeDelay(void);
void tickFadeTimer(void);
void finishSelection(void);
void handleMenuInput(void);
void updatePrompt(void);
void drawPrompt(void);
void drawPromptPanels(u8 selected, u8 alpha);
void drawLabelGroups(s16 x, s16 y, u8 selected, u8 alpha);
void drawLabelGroup(s16 x, s16 y, u8 selected, u8 alpha);
void openSelection(void);
void startSelectionFx(void);
void stopSelectionFx(void);
void finalizeExit(void);
void preDispatchGate(void);

#define GAME_FRONT_START_MASK         0x0800u
#define GAME_FRONT_POPUP_PENDING_MASK 0x00ffff00u
#define GAME_FRONT_POPUP_PENDING_OPEN 0x00020000u
#define GAME_FRONT_SELECTION_FX_TABLE PSX_PTR(const volatile u8, 0x80181ebau)
#define GAME_FRONT_STATE_HANDLERS     stateHandlerTable

#endif
