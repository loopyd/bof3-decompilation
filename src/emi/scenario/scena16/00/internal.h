#ifndef EMI_SCENA16_00_INTERNAL_H
#define EMI_SCENA16_00_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*Scena16Callback)(void);
typedef void (*Scena16RecordCallback)(void* record, u32 arg1);

extern vu8  SCENA16_D_80143C30;
extern vu16 SCENA16_D_80143C40;
extern vu8  SCENA16_D_80143BB0;
extern vu16 SCENA16_D_80143F00;
extern vu8  SCENA16_D_80143F03;
extern s32  SCENA16_D_80143F80;
extern vu8  SCENA16_D_80144F59;
extern vu8  SCENA16_D_80144F5A;
extern vu8  SCENA16_D_80144F5B;
extern vu8  SCENA16_D_80144F5C;
extern vu8  SCENA16_D_80144F5D;
extern vu8  SCENA16_D_80144F5E;
extern vu8  SCENA16_D_80144F5F;
extern vu8  SCENA16_D_80145029;
extern vu8  SCENA16_D_80145988;
extern vu8  SCENA16_D_80146254;
extern vu16 SCENA16_D_80146258;
extern vu8  SCENA16_D_80146866;
extern vu8  SCENA16_D_80146867;
extern vu32 SCENA16_D_80146864;
#define SCENA16_D_80146864_BYTE (*(vu8*)&SCENA16_D_80146864)
extern s8   SCENA16_D_80146872;
extern s8   SCENA16_D_80146874;
extern vu8  SCENA16_D_80146875;
extern vu16 SCENA16_D_80146876;
extern vu32 SCENA16_D_8014686C;
extern vu8  SCENA16_D_8014832E;
extern vu16 SCENA16_D_801492D8;
extern vu16 SCENA16_D_801492DA;
extern vu16 SCENA16_D_801492DC;
extern vu16 SCENA16_D_8014932C;
extern s32  SCENA16_D_8014930C;
extern vu32 SCENA16_D_80149314;
extern vu16 SCENA16_D_80149322;
extern s32  SCENA16_D_80147A90;
extern vu8  SCENA16_D_80143F1D;
extern vu16 SCENA16_D_80010004;
extern vu16 SCENA16_D_80010006;
extern vu16 SCENA16_D_80010008;
extern vu16 SCENA16_D_80010020;
extern vu16 SCENA16_D_80010022;
#define SCENA16_SELECTION_FX_TABLE CVPTR(u8, 0x80181ebau)
#define SCENA16_PALETTE_SRC        CVPTR(u16, 0x80033800u)
#define SCENA16_PALETTE_DST        VPTR(u16, 0x80037800u)
#define SCENA16_PTR_801F854C   ((Scena16Callback*)0x801f854cu)
#define SCENA16_PTR_801F8558   ((Scena16Callback*)0x801f8558u)
#define SCENA16_PTR_801F856C   ((Scena16RecordCallback*)0x801f856cu)

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

void func_801F6C90(void);
void func_801F6CCC(void);
void func_801F6D90(void);
void func_801F6E30(void);
void func_801F6EB0(void);
void func_801F6F30(void);
void func_801F7230(void);
void func_801F7790(void);
void func_801F7CC4(void);
void func_801F7144(void);
void func_801F7180(void);
void func_801F7188(void);
void func_801F8358(void* record);
s32  func_801F8398(void);
void func_801F83B0(u32 intensity);
s32  func_801F83A0(void);
s32  func_801F83A8(void);
void func_801F845C(void);
void func_801F84AC(void);
void func_801F8530(void);

#endif
