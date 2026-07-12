#ifndef BOF3_SRC_EMI_WORLD00_AREA030_04_INTERNAL_H
#define BOF3_SRC_EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"

#define WORLD00_AREA030_PRIMITIVE_PTR  VPPTR(u8, 0x8014598cu)
#define WORLD00_AREA030_SCRATCH_PTR    VPPTR(u8, 0x1f800044u)
#define WORLD00_AREA030_UI_CHAR_BUFFER VPTR(u8, 0x80145ad4u)

extern vu8  WORLD00_AREA030_GLOBAL_BYTE_3FC9;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_4002;
extern vu8  WORLD00_AREA030_GLOBAL_HALF_3FF2;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_3FFC;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_4000;
extern s16  WORLD00_AREA030_GLOBAL_HALF_4006;
extern vu32 WORLD00_AREA030_GLOBAL_WORD_3E6C;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_5E92;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_5EBA;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_5ED9;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_5EDA;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_5EDB;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_4011;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_4012;
extern vu8  WORLD00_AREA030_GLOBAL_BYTE_4013;
extern vu32 WORLD00_AREA030_GLOBAL_WORD_5EE0;
extern vu32 WORLD00_AREA030_GLOBAL_WORD_5EE4;
extern vu32 WORLD00_AREA030_GLOBAL_WORD_4018;
extern vu32 WORLD00_AREA030_GLOBAL_WORD_401C;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_5EE8;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_5EEA;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_4020;
extern vu16 WORLD00_AREA030_GLOBAL_HALF_4022;
extern s16  WORLD00_AREA030_GLOBAL_HALF_930E;
#define WORLD00_AREA030_SPRT_TABLE ((const volatile u8*)0x801e1d0cu)

void func_8014d290(void);
void func_8014d4e0(void);
void func_8014dd3c(s32 arg0);
void func_8014ff0c(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
int  func_8017e3f4(char* buffer, char* fmt, ...);
void func_801d195c(s16 arg0, s16 arg1);
void func_801d18cc(s16 arg0, s16 arg1, u8 arg2);
void func_801e0c80(s32 arg0, s32 arg1);
s32  func_801e0dcc(s32 arg0, s32 arg1, s16 arg2, s16 arg3);
s32  func_801d9534(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);
s32  func_80196070(void);
s16  func_8015477c(u16 arg0, u16 arg1);

void func_801d11c0(void);
void func_801d159c(s16 arg0, s16 arg1);
void func_801d1744(s16 arg0, s16 arg1, u8 arg2);
void func_801d1818(s16 arg0, s16 arg1, u8 arg2);
void func_801d1b88(s16 arg0, s16 arg1, s16 arg2, u8 arg3);
void func_801d2034(s16 arg0, s16 arg1, u8 arg2, s8 arg3);
void func_801d2ae0(void);
void func_801d2c34(s16 arg0, s16 arg1, s8 arg2, u8 arg3);
void func_801d3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, s8 arg5);
void func_801d3938(void);
void func_801d6a2c(void);
void func_801d6b28(s8 arg0);

#endif
