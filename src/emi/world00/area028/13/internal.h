#ifndef EMI_WORLD00_AREA028_13_INTERNAL_H
#define EMI_WORLD00_AREA028_13_INTERNAL_H

#include "bof3/bof3.h"

#include <rand.h>

typedef struct World00Area028Work {
  u8  unk_00[4];
  s16 field_04;
  s16 field_06;
  s16 field_08;
  s16 unk_0a;
  s16 field_0c;
  s16 field_0e;
} World00Area028Work;

extern volatile u16 WORLD00_AREA028_CENTER_X;
extern volatile u16 WORLD00_AREA028_CENTER_Y;
/* @kind: bss — base of the local 32-entry AREA028 work table; each 0x10-byte
 * slot holds one sprite work entry. */
extern u8                    world00_area028_work_table[1];
/* @kind: bss — current AREA028 work table cursor; seeded from the work table
 * base and walked by the seed/scan/refresh helpers. */
extern World00Area028Work*   world00_area028_work_cursor;

void func_801AFE18(void* arg0);
void func_80196070(void);
void func_801AFF04(const void* arg0, void* arg1);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A97C(void* arg0);
void func_8017A904(void* arg0, s32 arg1);
void func_8017AA30(void* arg0);
void func_8017C2D8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void func_8014E5A0(u8 arg0, u8 arg1);

void  func_801F2D3C(void);
void  world00_area028_work_table_seed(void);
void  world00_area028_sprite_slot_init(void* arg0);
void* world00_area028_free_slot_scan(void);
void  world00_area028_sprite_slots_refresh(void);
void  func_801F318C(s16 arg0);

#define WORLD00_AREA028_WORK_PTR (world00_area028_work_cursor)
#define WORLD00_AREA028_WORK_BASE     ((World00Area028Work*)0x800e4800u)
#define WORLD00_AREA028_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA028_RING_X(index)                                          \
  PSX_REF(volatile u16, 0x800e4a04u + ((u32)(index) * 4u))
#define WORLD00_AREA028_RING_Y(index)                                          \
  PSX_REF(volatile u16, 0x800e4a06u + ((u32)(index) * 4u))

#endif
