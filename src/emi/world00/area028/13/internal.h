#ifndef EMI_WORLD00_AREA028_13_INTERNAL_H
#define EMI_WORLD00_AREA028_13_INTERNAL_H

#include "bof3/bof3.h"

typedef struct World00Area028Work {
  u8  unk_00[4];
  s16 field_04;
  s16 field_06;
  s16 field_08;
  s16 unk_0a;
  s16 field_0c;
  s16 field_0e;
} World00Area028Work;

#define WORLD00_AREA028_WORK_PTR \
  PSX_REF(volatile World00Area028Work *, 0x801f3e00u)
#define WORLD00_AREA028_WORK_BASE     ((u8*)0x800e4800u)
#define WORLD00_AREA028_PRIMITIVE_PTR PSX_REF(volatile u8 *, 0x8014598cu)
#define WORLD00_AREA028_RING_X(index) \
  (*(volatile volatile u16*)(0x800e4a04u + ((u32)(index) * 4u)))
#define WORLD00_AREA028_RING_Y(index) \
  (*(volatile volatile u16*)(0x800e4a06u + ((u32)(index) * 4u)))
extern volatile u16 WORLD00_AREA028_CENTER_X;
extern volatile u16 WORLD00_AREA028_CENTER_Y;
void        func_801AFE18(void* arg0);
void        func_801AFF04(const void* arg0, void* arg1);
u16         func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void        func_8017A97C(void* arg0);
void        func_8017A904(void* arg0, s32 arg1);
void        func_8017AA30(void* arg0);
void        func_8017C2D8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void        func_8014E5A0(u8 arg0, u8 arg1);

void  func_801F2D3C(void);
void  func_801F2F5C(void);
void  func_801F2FB0(void* arg0);
void* func_801F3004(void);
void  func_801F3060(void);
void  func_801F318C(s16 arg0);

#endif
