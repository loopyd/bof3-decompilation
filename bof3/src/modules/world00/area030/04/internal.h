#ifndef BOF3_SRC_MODULES_WORLD00_AREA030_04_INTERNAL_H
#define BOF3_SRC_MODULES_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/modules/world00/area030/04.h"
#include "bof3/context.h"

#define WORLD00_AREA030_PRIMITIVE_PTR  (*(volatile u8**)0x8014598cu)
#define WORLD00_AREA030_SCRATCH_PTR    (*(volatile u8**)0x1f800044u)
#define WORLD00_AREA030_UI_CHAR_BUFFER ((volatile u8*)0x80145ad4u)

#define WORLD00_AREA030_GLOBAL_BYTE_3FC9 (*(volatile u8*)0x80143fc9u)
#define WORLD00_AREA030_GLOBAL_BYTE_4002 (*(volatile u8*)0x80144002u)
#define WORLD00_AREA030_GLOBAL_HALF_3FF2 (*(volatile u8*)0x80143ff2u)
#define WORLD00_AREA030_GLOBAL_HALF_3FFC (*(volatile u16*)0x80143ffcu)
#define WORLD00_AREA030_GLOBAL_HALF_4000 (*(volatile u16*)0x80144000u)
#define WORLD00_AREA030_GLOBAL_HALF_4006 (*(volatile s16*)0x80144006u)
#define WORLD00_AREA030_GLOBAL_WORD_3E6C (*(volatile u32*)0x80143e6cu)
#define WORLD00_AREA030_GLOBAL_BYTE_5E92 (*(volatile u8*)0x80145e92u)
#define WORLD00_AREA030_GLOBAL_BYTE_5EBA (*(volatile u8*)0x80145ebau)
#define WORLD00_AREA030_GLOBAL_BYTE_5ED9 (*(volatile u8*)0x80145ed9u)
#define WORLD00_AREA030_GLOBAL_BYTE_5EDA (*(volatile u8*)0x80145edau)
#define WORLD00_AREA030_GLOBAL_BYTE_5EDB (*(volatile u8*)0x80145edbu)
#define WORLD00_AREA030_GLOBAL_BYTE_4011 (*(volatile u8*)0x80144011u)
#define WORLD00_AREA030_GLOBAL_BYTE_4012 (*(volatile u8*)0x80144012u)
#define WORLD00_AREA030_GLOBAL_BYTE_4013 (*(volatile u8*)0x80144013u)
#define WORLD00_AREA030_GLOBAL_WORD_5EE0 (*(volatile u32*)0x80145ee0u)
#define WORLD00_AREA030_GLOBAL_WORD_5EE4 (*(volatile u32*)0x80145ee4u)
#define WORLD00_AREA030_GLOBAL_WORD_4018 (*(volatile u32*)0x80144018u)
#define WORLD00_AREA030_GLOBAL_WORD_401C (*(volatile u32*)0x8014401cu)
#define WORLD00_AREA030_GLOBAL_HALF_5EE8 (*(volatile u16*)0x80145ee8u)
#define WORLD00_AREA030_GLOBAL_HALF_5EEA (*(volatile u16*)0x80145eeau)
#define WORLD00_AREA030_GLOBAL_HALF_4020 (*(volatile u16*)0x80144020u)
#define WORLD00_AREA030_GLOBAL_HALF_4022 (*(volatile u16*)0x80144022u)
#define WORLD00_AREA030_GLOBAL_HALF_930E (*(volatile s16*)0x8014930eu)
#define WORLD00_AREA030_SPRT_TABLE       ((const volatile u8*)0x801e1d0cu)

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

#endif
