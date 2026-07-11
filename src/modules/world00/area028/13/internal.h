#ifndef BOF3_SRC_MODULES_WORLD00_AREA028_13_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA028_13_INTERNAL_H

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

#define WORLD00_AREA028_WORK_PTR      VPPTR(World00Area028Work, 0x801f3e00u)
#define WORLD00_AREA028_WORK_BASE     ((u8*)0x800e4800u)
#define WORLD00_AREA028_PRIMITIVE_PTR VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA028_RING_X(index) \
  (*(volatile volatile u16*)(0x800e4a04u + ((u32)(index) * 4u)))
#define WORLD00_AREA028_RING_Y(index) \
  (*(volatile volatile u16*)(0x800e4a06u + ((u32)(index) * 4u)))
extern vu16 WORLD00_AREA028_CENTER_X;
extern vu16 WORLD00_AREA028_CENTER_Y;
void        func_801afe18(void* arg0);
void        func_801aff04(const void* arg0, void* arg1);
u16         func_8017a620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void        func_8017a97c(void* arg0);
void        func_8017a904(void* arg0, s32 arg1);
void        func_8017aa30(void* arg0);
void        func_8017c2d8(void* arg0, s32 arg1, s32 arg2, s32 arg3, void* arg4);
void        func_8014e5a0(u8 arg0, u8 arg1);

void  func_801f2d3c(void);
void  func_801f2f5c(void);
void  func_801f2fb0(void* arg0);
void* func_801f3004(void);
void  func_801f3060(void);
void  func_801f318c(s16 arg0);

#endif
