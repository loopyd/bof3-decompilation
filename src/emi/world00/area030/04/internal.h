#ifndef EMI_WORLD00_AREA030_04_INTERNAL_H
#define EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared mode byte; stored small mode values (4/6/7/...) and compared
 * against them by the area030 mode handlers. */
extern u8  modeByte;      /* @source 0x80144125 @kind bss */
extern u8  D_80144286;    /* @source 0x80144286 @kind unknown */
extern u8  D_80145E93;    /* @source 0x80145E93 @kind unknown */
extern void (*D_801E2000[])(void); /* @source 0x801E2000 @kind table */
extern void (*D_801E2014[])(void); /* @source 0x801E2014 @kind table */
extern void (*D_801E2048[])(void); /* @source 0x801E2048 @kind table */
extern void (*D_801E2074[])(void); /* @source 0x801E2074 @kind table */
extern void (*D_801E20C4[])(void); /* @source 0x801E20C4 @kind table */
extern void (*D_801E21E0[])(void); /* @source 0x801E21E0 @kind table */
extern void (*D_801E2210[])(void); /* @source 0x801E2210 @kind table */
extern void (*D_801E221C[])(void); /* @source 0x801E221C @kind table */
extern void (*D_801E22E4[])(void); /* @source 0x801E22E4 @kind table */
extern u8  D_801E2384[];  /* @source 0x801E2384 @kind unknown */
extern u8  D_801E2388[];  /* @source 0x801E2388 @kind unknown */
extern u8  D_801E238C[];  /* @source 0x801E238C @kind unknown */
extern u8  D_801E2390[];  /* @source 0x801E2390 @kind unknown */
extern u8* D_8014598C;    /* @source 0x8014598C @kind unknown */
/* Scratchpad work-record cursor cell; reloaded per store group by the
 * AREA030 handlers (volatile cell, plain RAM pointee). */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind bss */

/* Scoped companion-call ABI for func_801E0C20; not a callback contract. */
void func_800F500C(void);
void func_8014D290(void);
void func_8014D4E0(void);
void func_8014DD3C(s32 arg0);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
int  func_8017E3F4(char* buffer, char* fmt, ...);
void func_801D195C(s16 arg0, s16 arg1);
void func_801D18CC(s16 arg0, s16 arg1, u8 arg2);
void submitTpageDrawMode(s32 arg0, s32 arg1);
u8*  func_801E0DCC(s32 arg0, s32 arg1, s16 arg2, s16 arg3);
s32  func_801D9534(s16 arg0, u16 arg1, s16 arg2, s16 arg3, s32 arg4);
s32  func_80196070(void);
s16  func_8015477C(u16 arg0, u16 arg1);

void func_801D11C0(void);
void func_801D9B14(void);
void func_801D9B58(void);
void func_801D9CF4(void);
void func_801D5C48(void);
void func_801D6000(void);
void func_801D6554(void);
void func_801D159C(s16 arg0, s16 arg1);
void func_801D1744(s16 arg0, s16 arg1, u8 arg2);
void func_801D1818(s16 arg0, s16 arg1, u8 arg2);
void func_801D1B88(s16 arg0, s16 arg1, s16 arg2, u8 arg3);
void queueIconStrip(s16 arg0, s16 arg1, u8 arg2, s8 arg3);
void func_801D2AE0(void);
void func_801D2C34(s16 arg0, s16 arg1, s8 arg2, u8 arg3);
void func_801D3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, s8 arg5);
void advancePanelScroll(void);
void func_801D6F08(void);
void seedMenuScratch(void);
void func_801D6B28(s8 arg0);
void func_801D6EC4(void);
void advanceStepMode6(void);
void clearFlagState10(void);
void dispatchCompanion(void);
void func_801DCC74(void);
void submitPanelPair(s16 arg0, s16 arg1);
void configureSpriteClut(s16 arg0, s16 arg1, u8 arg2);
void appendDimTile(void);

#define WORLD00_AREA030_SCRATCH_PTR PSX_REF(volatile u8*, 0x1f800044u)

#endif
