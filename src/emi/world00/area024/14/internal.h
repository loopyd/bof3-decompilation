#ifndef EMI_WORLD00_AREA024_14_INTERNAL_H
#define EMI_WORLD00_AREA024_14_INTERNAL_H

#include <rand.h>

#include "bof3/bof3.h"

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

extern volatile u8  D_80147A58;
extern volatile u16 WORLD00_AREA024_GLOBAL_HALF_3E6C;
extern u16          D_801490A8;
/* @kind: bss — current 0x28-byte work entry cursor; seeded from the work base
 * and walked by the dispatcher. */
extern u8*                              world00_area024_work_cursor;
/* @kind: rodata — four-entry per-mode handler table dispatched by the work
 * dispatcher on each active entry's byte +0x01. */
extern const World00Area024Handler      world00_area024_state_table[];

void func_8015B410(void* arg0);
void func_8015B4B0(void* arg0);
void func_80196070(void);
void func_801AFF64(void* arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
u16  func_8017A6F0(s32 arg0, s32 arg1);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A97C(void* arg0);
void func_8017A904(void* arg0, s32 arg1);
void func_8017A9B8(void* arg0);
void func_8017AAE8(void* arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_801F2DF8(const void* arg0);
void func_801F2FD4(void* arg0);
void func_801F3080(void);
s32  world00_area024_dispatch_work_states(void);
void func_801F362C(void);
void func_801F3708(void* arg0, const void* arg1, s16* arg2);
void func_801F3944(const void* arg0);
void func_801F3BE4(void* arg0);
void world00_area024_spin_work_init(void);
void world00_area024_spin_draw(void);
s32  func_801F3E48(u8 arg0);
s16  func_801F4158(const s16* arg0, const s16* arg1, const s16* arg2);

#define WORLD00_AREA024_PRIMITIVE_PTR  PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA024_WORK_PTR       PSX_REF(volatile u8*, 0x801f5b00u)
#define WORLD00_AREA024_WORK_BASE      PSX_PTR(u8, 0x800e4800u)
#define WORLD00_AREA024_SPIN_WORK_BASE PSX_PTR(u8, 0x800e5000u)
#define WORLD00_AREA024_STATE_BASE     PSX_PTR(volatile s16, 0x800e4940u)
#define WORLD00_AREA024_STATE_OFFSET   PSX_PTR(volatile s16, 0x800e4944u)
#define WORLD00_AREA024_VERTEX_DST     PSX_PTR(volatile u8, 0x800e4bc8u)
#define WORLD00_AREA024_VERTEX_SRC     PSX_PTR(const volatile u8, 0x80147aa8u)
#define WORLD00_AREA024_SCRATCH_REMAP  PSX_PTR(volatile u8, 0x80147a58u)
#define WORLD00_AREA024_PTR_7AA8       PSX_REF(volatile u8*, 0x80147aa8u)
#define WORLD00_AREA024_PTR_7AAC       PSX_REF(volatile u8*, 0x80147aacu)
#define WORLD00_AREA024_STATE_TABLE                                            \
  PSX_PTR(const volatile World00Area024Handler, 0x801f4214u)
#define WORLD00_AREA024_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area024Scratch*, 0x1f800044u)
#define WORLD00_AREA024_SCRATCH_BYTE_09 PSX_REF(volatile u8, 0x1f80004du)

#endif
