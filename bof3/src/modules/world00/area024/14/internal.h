#ifndef BOF3_SRC_MODULES_WORLD00_AREA024_14_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA024_14_INTERNAL_H

#include "bof3/bof3.h"

#include <rand.h>

typedef void (*World00Area024Handler)(void);

typedef struct World00Area024SpriteWork {
  u8     field_00;
  u8     field_01;
  u8     field_02;
  u8     field_03;
  s32    field_04;
  s32    field_08;
  s32    field_0c;
  u8     unk_10[4];
  VECTOR field_14;
  u16    field_24;
} World00Area024SpriteWork;

typedef struct World00Area024Scratch {
  u8  unk_00[0x34];
  s32 field_34;
  s32 field_38;
  s32 field_3c;
} World00Area024Scratch;

typedef struct World00Area024SpinWork {
  s32     field_00;
  s32     field_04;
  s32     field_08;
  u8      unk_0c[4];
  SVECTOR field_10;
  SVECTOR field_18;
  s16     field_20;
  s16     field_22;
  s16     field_24;
  s16     unk_26;
  s16     field_28;
  s16     field_2a;
} World00Area024SpinWork;

#define WORLD00_AREA024_PRIMITIVE_PTR VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA024_WORK_PTR      VPPTR(u8, 0x801f5b00u)
#define WORLD00_AREA024_WORK_BASE     ((u8*)0x800e4800u)
#define WORLD00_AREA024_SPIN_WORK_BASE ((u8*)0x800e5000u)
#define WORLD00_AREA024_STATE_BASE    VPTR(s16, 0x800e4940u)
#define WORLD00_AREA024_STATE_OFFSET  VPTR(s16, 0x800e4944u)
#define WORLD00_AREA024_VERTEX_DST    VPTR(u8, 0x800e4bc8u)
#define WORLD00_AREA024_VERTEX_SRC    ((const volatile u8*)0x80147aa8u)
#define WORLD00_AREA024_SCRATCH_REMAP VPTR(u8, 0x80147a58u)
#define WORLD00_AREA024_PTR_7AA8      VPPTR(u8, 0x80147aa8u)
#define WORLD00_AREA024_PTR_7AAC      VPPTR(u8, 0x80147aacu)
extern vu16 WORLD00_AREA024_GLOBAL_HALF_3E6C;
#define WORLD00_AREA024_STATE_TABLE \
  ((World00Area024Handler const volatile*)0x801f4214u)
#define WORLD00_AREA024_SCRATCH_PTR \
  VPPTR(World00Area024Scratch, 0x1f800044u)
#define WORLD00_AREA024_SCRATCH_BYTE_09 (*(volatile volatile u8 *)(0x1f800044u + 9u))

void func_8015b410(void* arg0);
void func_8015b4b0(void* arg0);
void func_801aff64(void* arg0);
void func_801afe18(void* arg0);
void func_801aff04(const void* arg0, void* arg1);
void func_801affd8(const void* arg0, void* arg1, void* arg2);
u16  func_8017a6f0(s32 arg0, s32 arg1);
u16  func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017a97c(void* arg0);
void func_8017a904(void* arg0, s32 arg1);
void func_8017a9b8(void* arg0);
void func_8017aae8(void* arg0);
void func_80155a08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* from old bof3/include/bof3/modules/world00/area024/14.h */
void func_801f2df8(const void* arg0);
void func_801f2fd4(void* arg0);
void func_801f3080(void);
s32  func_801f30ec(void);
void func_801f362c(void);
void func_801f3708(void* arg0, const void* arg1, s16* arg2);
void func_801f3944(const void* arg0);
void func_801f3be4(void* arg0);
void func_801f3d0c(void);
void func_801f3d5c(void);
s32  func_801f3e48(u8 arg0);
s16  func_801f4158(const s16* arg0, const s16* arg1, const s16* arg2);

#endif
