#ifndef BOF3_SRC_MODULES_SCENA16_00_INTERNAL_H
#define BOF3_SRC_MODULES_SCENA16_00_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Scena16Callback)(void);
typedef void (*Scena16RecordCallback)(void* record, u32 arg1);

extern vu8  BOF3_SCENA16_DAT_80143c30;
extern vu16 BOF3_SCENA16_DAT_80143c40;
extern vu8  BOF3_SCENA16_DAT_80143bb0;
extern vu16 BOF3_SCENA16_DAT_80143f00;
extern vu8  BOF3_SCENA16_DAT_80143f03;
extern s32  BOF3_SCENA16_DAT_80143f80;
extern vu8  BOF3_SCENA16_DAT_80144f59;
extern vu8  BOF3_SCENA16_DAT_80144f5a;
extern vu8  BOF3_SCENA16_DAT_80144f5b;
extern vu8  BOF3_SCENA16_DAT_80144f5c;
extern vu8  BOF3_SCENA16_DAT_80144f5d;
extern vu8  BOF3_SCENA16_DAT_80144f5e;
extern vu8  BOF3_SCENA16_DAT_80144f5f;
extern vu8  BOF3_SCENA16_DAT_80145029;
extern vu8  BOF3_SCENA16_DAT_80145988;
extern vu8  BOF3_SCENA16_DAT_80146254;
extern vu16 BOF3_SCENA16_DAT_80146258;
extern vu8  BOF3_SCENA16_DAT_80146866;
extern vu8  BOF3_SCENA16_DAT_80146867;
extern vu8  BOF3_SCENA16_DAT_80146864_BYTE;
extern vu32 BOF3_SCENA16_DAT_80146864;
extern s8   BOF3_SCENA16_DAT_80146872;
extern s8   BOF3_SCENA16_DAT_80146874;
extern vu8  BOF3_SCENA16_DAT_80146875;
extern vu16 BOF3_SCENA16_DAT_80146876;
extern vu32 BOF3_SCENA16_DAT_8014686c;
extern vu8  BOF3_SCENA16_DAT_8014832e;
extern vu16 BOF3_SCENA16_DAT_801492d8;
extern vu16 BOF3_SCENA16_DAT_801492da;
extern vu16 BOF3_SCENA16_DAT_801492dc;
extern vu16 BOF3_SCENA16_DAT_8014932c;
extern s32  BOF3_SCENA16_DAT_8014930c;
extern vu32 BOF3_SCENA16_DAT_80149314;
extern vu16 BOF3_SCENA16_DAT_80149322;
extern s32  BOF3_SCENA16_DAT_80147a90;
extern vu8  BOF3_SCENA16_DAT_80143f1d;
extern vu16 BOF3_SCENA16_DAT_80010004;
extern vu16 BOF3_SCENA16_DAT_80010006;
extern vu16 BOF3_SCENA16_DAT_80010008;
extern vu16 BOF3_SCENA16_DAT_80010020;
extern vu16 BOF3_SCENA16_DAT_80010022;
#define BOF3_SCENA16_SELECTION_FX_TABLE CVPTR(u8, 0x80181ebau)
#define BOF3_SCENA16_PALETTE_SRC        CVPTR(u16, 0x80033800u)
#define BOF3_SCENA16_PALETTE_DST        VPTR(u16, 0x80037800u)
#define BOF3_SCENA16_PTR_FUN_801f854c   ((Scena16Callback*)0x801f854cu)
#define BOF3_SCENA16_PTR_FUN_801f8558   ((Scena16Callback*)0x801f8558u)
#define BOF3_SCENA16_PTR_FUN_801f856c   ((Scena16RecordCallback*)0x801f856cu)

void game_stop_selection_fx(u32 effect_group, s32 effect_id);
void game_queue_frontend_cue(u32 cue_id);
void func_80150224(u32 arg0);
void func_80154fd8(u32 arg0);
void func_80154698(void);
void func_8014f800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_8015b580(u32 arg0, s32 arg1);
s32  func_8015b5d4(u32 arg0, s32 arg1);
void func_8015c088(void);
void func_8015c100(void);
void func_80166e88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8016c0c0(s32 arg0, s32 arg1);
void func_80161c20(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80161cd0(u8 selection_id, s32 cue_level, s32 cue_shape);
void func_80161bbc(u32 slot_id);
u8   func_8019601c(void);
void func_8019fa28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801be1b0(u32 arg0);
void func_801c1df0(u32 arg0);
void func_801c601c(u32 arg0);
void func_801c187c(s32 arg0);

void func_801f6c90(void);
void func_801f6ccc(void);
void func_801f6d90(void);
void func_801f6e30(void);
void func_801f6eb0(void);
void func_801f6f30(void);
void func_801f7230(void);
void func_801f7790(void);
void func_801f7cc4(void);
void func_801f7144(void);
void func_801f7180(void);
void func_801f7188(void);
void func_801f8358(void* record);
s32  func_801f8398(void);
void func_801f83b0(u32 intensity);
s32  func_801f83a0(void);
s32  func_801f83a8(void);
void func_801f845c(void);
void func_801f84ac(void);
void func_801f8530(void);

#endif
