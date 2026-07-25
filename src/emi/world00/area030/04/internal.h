#ifndef EMI_WORLD00_AREA030_04_INTERNAL_H
#define EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_3FC9;
extern u8           D_80145E93;
extern u8           D_801E2384[];
extern u8           D_801E2388[];
extern u8           D_801E238C[];
extern u8           D_801E2390[];
extern u8*          D_8014598C;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_4002;
extern volatile u8  WORLD00_AREA030_GLOBAL_HALF_3FF2;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_3FFC;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_4000;
extern s16          WORLD00_AREA030_GLOBAL_HALF_4006;
extern volatile u32 WORLD00_AREA030_GLOBAL_WORD_3E6C;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_5E92;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_5EBA;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_5ED9;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_5EDA;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_5EDB;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_4011;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_4012;
extern volatile u8  WORLD00_AREA030_GLOBAL_BYTE_4013;
extern volatile u32 WORLD00_AREA030_GLOBAL_WORD_5EE0;
extern volatile u32 WORLD00_AREA030_GLOBAL_WORD_5EE4;
extern volatile u32 WORLD00_AREA030_GLOBAL_WORD_4018;
extern volatile u32 WORLD00_AREA030_GLOBAL_WORD_401C;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_5EE8;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_5EEA;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_4020;
extern volatile u16 WORLD00_AREA030_GLOBAL_HALF_4022;
extern s16          WORLD00_AREA030_GLOBAL_HALF_930E;

/* Scoped companion-call ABI for func_801E0C20; not a callback contract. */
void func_800F500C(void);
void func_8014D290(void);
void func_8014D4E0(void);
void func_8014DD3C(s32 arg0);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
int  func_8017E3F4(char* buffer, char* fmt, ...);
void func_801D195C(s16 arg0, s16 arg1);
void func_801D18CC(s16 arg0, s16 arg1, u8 arg2);
void func_801E0C80(s32 arg0, s32 arg1);
u8*  func_801E0DCC(s32 arg0, s32 arg1, s16 arg2, s16 arg3);
s32  func_801D9534(s16 arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4);
s32  func_80196070(void);
s16  func_8015477C(u16 arg0, u16 arg1);

void func_801D11C0(void);
void func_801D159C(s16 arg0, s16 arg1);
void func_801D1744(s16 arg0, s16 arg1, u8 arg2);
void func_801D1818(s16 arg0, s16 arg1, u8 arg2);
void func_801D1B88(s16 arg0, s16 arg1, s16 arg2, u8 arg3);
void func_801D2034(s16 arg0, s16 arg1, u8 arg2, s8 arg3);
void func_801D2AE0(void);
void func_801D2C34(s16 arg0, s16 arg1, s8 arg2, u8 arg3);
void func_801D3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, s8 arg5);
void func_801D3938(void);
void func_801D6A2C(void);
void func_801D6B28(s8 arg0);

#define WORLD00_AREA030_PRIMITIVE_PTR  D_8014598C
#define WORLD00_AREA030_SCRATCH_PTR    PSX_REF(volatile u8*, 0x1f800044u)
#define WORLD00_AREA030_UI_CHAR_BUFFER PSX_PTR(volatile u8, 0x80145ad4u)
#define WORLD00_AREA030_SPRT_TABLE     PSX_PTR(const volatile u8, 0x801e1d0cu)

#endif
