#ifndef EMI_SCENA16_00_INTERNAL_H
#define EMI_SCENA16_00_INTERNAL_H

#include "bof3/bof3.h"
#include "frontend/state.h"
#include "frontend/selection.h"
#include "gpu/palette.h"

typedef void (*Scena16Callback)(void);
typedef void (*Scena16RecordCallback)(void* record, u32 arg1);

/* @source 0x80143C30 @kind unknown */
extern volatile u8  D_80143C30;
/* @source 0x80143C40 @kind unknown */
extern volatile u16 D_80143C40;
/* @source 0x80143F03 @kind unknown */
extern u8  D_80143F03;
/* @source 0x80143F80 @kind unknown */
extern s32          D_80143F80;
/* @source 0x80145029 @kind unknown */
extern volatile u8  D_80145029;
/* @source 0x80145988 @kind unknown */
extern volatile u8  D_80145988;
/* @source 0x80146258 @kind unknown */
extern volatile u16 D_80146258;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x80146864 @kind unknown */
extern volatile u8 g_ScenarioProgress;
/* @source 0x80146874 @kind unknown */
extern volatile s8  D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8  D_80146875;
/* @source 0x80146876 @kind unknown */
extern u16 D_80146876;
/* @source 0x8014686C @kind unknown */
extern volatile u32 D_8014686C;
/* @source 0x8014832E @kind unknown */
extern u8  D_8014832E;
/* @source 0x801492D8 @kind unknown */
extern volatile u16 D_801492D8;
/* @source 0x801492DA @kind unknown */
extern volatile u16 D_801492DA;
/* @source 0x801492DC @kind unknown */
extern volatile u16 D_801492DC;
/* @source 0x8014932C @kind unknown */
extern volatile u16 D_8014932C;
/* @source 0x8014930C @kind unknown */
extern s32          D_8014930C;
/* @source 0x80149314 @kind unknown */
extern volatile u32 D_80149314;
/* @source 0x80149322 @kind unknown */
extern volatile u16 D_80149322;
/* @source 0x80147A90 @kind unknown */
extern s32          D_80147A90;
/* @source 0x80143F1D @kind unknown */
extern volatile u8  D_80143F1D;
/* @source 0x80010004 @kind unknown */
extern volatile u16 D_80010004;
/* @source 0x80010006 @kind unknown */
extern volatile u16 D_80010006;
/* @source 0x80010008 @kind unknown */
extern volatile u16 D_80010008;
/* @source 0x80010020 @kind unknown */
extern volatile u16 D_80010020;
/* @source 0x80010022 @kind unknown */
extern volatile u16 D_80010022;
/* @source 0x80145EC4 @kind unknown */
extern volatile u32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern volatile u32 D_80145EC8;
/* @source 0x80149308 @kind unknown */
extern volatile u32 D_80149308;
/* @source 0x801F854C @kind table — primary state handlers indexed by bank byte 0x6872:
 * boot, area route, secondary dispatch. */
extern Scena16Callback primaryStateTable[];
/* @source 0x801F8558 @kind table — secondary state handlers indexed by D_80146874. */
extern Scena16Callback secondaryStateTable[];
/* @source 0x801F856C @kind table — record callbacks indexed by record byte 0x7a. */
extern Scena16RecordCallback recordCallbackTable[];
/* @source 0x80181EBA @kind unknown */
/* @source 0x80181EBA @kind unknown */
extern const u8 D_80181EBA[];

void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void game_queue_frontend_cue(u32 cue_id);
void func_80150224(u32 arg0);
void func_80154FD8(u32 arg0);
void func_80154698(void);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_8015B580(u32 arg0, s32 arg1);
s32  func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_8015C100(void);
void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8016C0C0(s32 arg0, s32 arg1);
void func_80161BBC(u32 slot_id);
u8   func_8019601C(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);
void func_801C1DF0(u32 arg0);
void func_801C601C(u32 arg0);
void func_801C187C(s32 arg0);

void dispatchPrimaryState(void);
void bootPrimaryState(void);
void func_801F6D90(void);
void seedRouteEnterState3(void);
void seedRouteEnterState2(void);
void func_801F6F30(void);
void func_801F7230(void);
void func_801F7790(void);
void func_801F7CC4(void);
void dispatchSecondaryState(void);
void noopSecondaryState(void);
void finalizeSecondaryPath(void);
void dispatchRecordCallback(void* record);
s32  returnZero(void);
void clampPaletteChannels(u32 intensity);
s32  returnZero2(void);
s32  returnZero3(void);
void copyPaletteBlock(void);
void resetEffectBank(void);
void noopRecordHandler(void);

#define SCENA16_PALETTE_SRC        PSX_PTR(const volatile u16, 0x80033800u)
#define SCENA16_PALETTE_DST        PSX_PTR(volatile u16, 0x80037800u)
#define SCENA16_VRAM_BASE          (0x80010000u)

#endif
