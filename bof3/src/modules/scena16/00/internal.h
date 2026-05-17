#ifndef BOF3_SRC_MODULES_SCENA16_00_INTERNAL_H
#define BOF3_SRC_MODULES_SCENA16_00_INTERNAL_H

#include "bof3/context.h"
#include "bof3/core/callback_scheduler.h"
#include "bof3/core/emi/loader.h"
#include "bof3/core/game_front.h"
#include "bof3/modules/scena16/00.h"

typedef void (*Scena16Callback)(void);
typedef void (*Scena16RecordCallback)(void* record, u32 arg1);

#define BOF3_SCENA16_DAT_80143c30       (*(vu8*)0x80143c30u)
#define BOF3_SCENA16_DAT_80143c40       (*(vu16*)0x80143c40u)
#define BOF3_SCENA16_DAT_80143bb0       (*(vu8*)0x80143bb0u)
#define BOF3_SCENA16_DAT_80143f00       (*(vu16*)0x80143f00u)
#define BOF3_SCENA16_DAT_80143f03       (*(vu8*)0x80143f03u)
#define BOF3_SCENA16_DAT_80143f80       (*(vs32*)0x80143f80u)
#define BOF3_SCENA16_DAT_80144f59       (*(vu8*)0x80144f59u)
#define BOF3_SCENA16_DAT_80144f5a       (*(vu8*)0x80144f5au)
#define BOF3_SCENA16_DAT_80144f5b       (*(vu8*)0x80144f5bu)
#define BOF3_SCENA16_DAT_80144f5c       (*(vu8*)0x80144f5cu)
#define BOF3_SCENA16_DAT_80144f5d       (*(vu8*)0x80144f5du)
#define BOF3_SCENA16_DAT_80144f5e       (*(vu8*)0x80144f5eu)
#define BOF3_SCENA16_DAT_80144f5f       (*(vu8*)0x80144f5fu)
#define BOF3_SCENA16_DAT_80145029       (*(vu8*)0x80145029u)
#define BOF3_SCENA16_DAT_80145988       (*(vu8*)0x80145988u)
#define BOF3_SCENA16_DAT_80146254       (*(vu8*)0x80146254u)
#define BOF3_SCENA16_DAT_80146258       (*(vu16*)0x80146258u)
#define BOF3_SCENA16_DAT_80146866       (*(vu8*)0x80146866u)
#define BOF3_SCENA16_DAT_80146867       (*(vu8*)0x80146867u)
#define BOF3_SCENA16_DAT_80146864_BYTE  (*(vu8*)0x80146864u)
#define BOF3_SCENA16_DAT_80146864       (*(vu32*)0x80146864u)
#define BOF3_SCENA16_DAT_80146872       (*(vs8*)0x80146872u)
#define BOF3_SCENA16_DAT_80146874       (*(vs8*)0x80146874u)
#define BOF3_SCENA16_DAT_80146875       (*(vu8*)0x80146875u)
#define BOF3_SCENA16_DAT_80146876       (*(vu16*)0x80146876u)
#define BOF3_SCENA16_DAT_8014686c       (*(vu32*)0x8014686cu)
#define BOF3_SCENA16_DAT_8014832e       (*(vu8*)0x8014832eu)
#define BOF3_SCENA16_DAT_801492d8       (*(vu16*)0x801492d8u)
#define BOF3_SCENA16_DAT_801492da       (*(vu16*)0x801492dau)
#define BOF3_SCENA16_DAT_801492dc       (*(vu16*)0x801492dcu)
#define BOF3_SCENA16_DAT_8014932c       (*(vu16*)0x8014932cu)
#define BOF3_SCENA16_DAT_8014930c       (*(vs32*)0x8014930cu)
#define BOF3_SCENA16_DAT_80149314       (*(vu32*)0x80149314u)
#define BOF3_SCENA16_DAT_80149322       (*(vu16*)0x80149322u)
#define BOF3_SCENA16_DAT_80147a90       (*(vs32*)0x80147a90u)
#define BOF3_SCENA16_DAT_80143f1d       (*(vu8*)0x80143f1du)
#define BOF3_SCENA16_DAT_80010004       (*(const vu16*)0x80010004u)
#define BOF3_SCENA16_DAT_80010006       (*(const vu16*)0x80010006u)
#define BOF3_SCENA16_DAT_80010008       (*(const vu16*)0x80010008u)
#define BOF3_SCENA16_DAT_80010020       (*(const vu16*)0x80010020u)
#define BOF3_SCENA16_DAT_80010022       (*(const vu16*)0x80010022u)
#define BOF3_SCENA16_SELECTION_FX_TABLE ((const vu8*)0x80181ebau)
#define BOF3_SCENA16_PALETTE_SRC        ((const vu16*)0x80033800u)
#define BOF3_SCENA16_PALETTE_DST        ((vu16*)0x80037800u)
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

#endif
