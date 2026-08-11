#ifndef EMI_WORLD00_AREA030_04_INTERNAL_H
#define EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared mode byte; stored small mode values (4/6/7/...) and compared
 * against them by the area030 mode handlers. */
extern u16 D_80143C40;    /* @source 0x80143C40 @kind unknown */
extern s32 D_80143E6C;    /* shared frame counter */
extern u16 D_80143B92;    /* @source 0x80143B92 @kind unknown */
extern u8  D_8014832E;    /* @source 0x8014832E @kind unknown */
extern u8  D_80149332;    /* @source 0x80149332 @kind unknown */
extern u16 D_8014932E;    /* @source 0x8014932E @kind unknown */
extern u32 D_8014930C;    /* @source 0x8014930C @kind unknown */
extern u8  D_8014403D;    /* @source 0x8014403D @kind unknown */
extern u8  D_80144281;    /* @source 0x80144281 @kind unknown */
extern u8  modeByte;      /* @source 0x80144125 @kind bss */
extern u8  D_8014412A;    /* @source 0x8014412A @kind unknown */
extern u8  D_8014412B;    /* @source 0x8014412B @kind unknown */
extern u8  D_80144199[];  /* @source 0x80144199 @kind unknown */
extern u8  D_8014419E;    /* @source 0x8014419E @kind unknown */
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
extern void (*D_801E2248[])(void); /* @source 0x801E2248 @kind table */
extern void (*D_801E2258[])(void); /* @source 0x801E2258 @kind table */
extern void (*D_801E2268[])(void); /* @source 0x801E2268 @kind table */
extern void (*D_801E22A4[])(void); /* @source 0x801E22A4 @kind table */
extern void (*D_801E22B0[])(void); /* @source 0x801E22B0 @kind table */
extern void (*D_801E22BC[])(void); /* @source 0x801E22BC @kind table */
extern void (*D_801E22C8[])(void); /* @source 0x801E22C8 @kind table */
extern void (*D_801E22D0[])(void); /* @source 0x801E22D0 @kind table */
extern void (*D_801E22F0[])(void); /* @source 0x801E22F0 @kind table */
extern void (*D_801E22FC[])(void); /* @source 0x801E22FC @kind table */
extern void (*D_801E2304[])(void); /* @source 0x801E2304 @kind table */
extern void (*D_801E2340[])(void); /* @source 0x801E2340 @kind table */
extern void (*D_801E2348[])(void); /* @source 0x801E2348 @kind table */
extern void (*D_801E22E4[])(void); /* @source 0x801E22E4 @kind table */
extern u8  D_801E2384[];  /* @source 0x801E2384 @kind unknown */
extern u8  D_801E2388[];  /* @source 0x801E2388 @kind unknown */
extern u8  D_801E238C[];  /* @source 0x801E238C @kind unknown */
extern u8  D_801E2390[];  /* @source 0x801E2390 @kind unknown */
extern u8* g_PrimCursor;    /* @source 0x8014598C @kind unknown */
extern s32 D_8014421C;     /* @source 0x8014421C @kind unknown */
extern u16 D_80145AA8;     /* @source 0x80145AA8 @kind unknown */
extern u8  D_801E31F0;     /* @source 0x801E31F0 @kind unknown */
extern u8  D_801E31F4;     /* @source 0x801E31F4 @kind unknown */
extern u8  D_801E31F8;     /* @source 0x801E31F8 @kind unknown */
extern u8 D_80145026;      /* @source 0x80145026 @kind unknown */
extern u8 D_80145028;      /* @source 0x80145028 @kind unknown */
extern u8 D_801E2720[];    /* @source 0x801E2720 @kind unknown */
extern u8 D_801E2748[];    /* @source 0x801E2748 @kind unknown */
extern u8 D_801E3208;      /* @source 0x801E3208 @kind unknown */
extern s8* D_801E320C;     /* @source 0x801E320C @kind unknown */
extern u8* D_801E3210;     /* @source 0x801E3210 @kind unknown */

typedef struct Area030SlotState {
  s32 value;
  u8 pad_04[0x94];
} Area030SlotState;

extern Area030SlotState D_801468A4[]; /* @source 0x801468A4 @kind unknown */

typedef struct Area030Range {
  u8 pad_00[0x8C];
  s16 limit_8C;
  s16 value_8E;
} Area030Range;

extern Area030Range* D_80146884; /* @source 0x80146884 @kind unknown */

typedef struct SpriteGeometry {
  s32 clut_x;
  s32 clut_y;
  u16 width;
  u16 height;
  u8 u;
  u8 v;
  u8 pad_0E[2];
} SpriteGeometry;

extern SpriteGeometry D_801E2424[]; /* @source 0x801E2424 @kind table */
/* Scratchpad work-record cursor cell; reloaded per store group by the
 * AREA030 handlers (volatile cell, plain RAM pointee). */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind bss */

/* Scoped companion-call ABI for func_801E0C20; not a callback contract. */
void dispatchArea030CompanionHandler(void); /* @source 0x800F500C */
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
void func_80196070(void);
s16  func_8015477C(u16 arg0, u16 arg1);

void func_801D11C0(void);
void func_801D9B14(void);
void func_801D9B58(void);
void func_801D9C9C(void);
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
void func_801E026C(s8 arg0);
void func_801DAE3C(void);
void dispatchCompanion(void);
void func_801DFFA8(void);
void func_801DCC74(void);
void func_801DCCB0(void);
void func_801DDFD4(void);
void func_801DDE94(s32 state);
void func_801DC474(void);
void func_801DC590(void);
void func_801DC64C(void);
void func_801DC708(void);
void func_801DC7EC(void);
void func_801DA8E8(void);
void func_801DAE84(void);
void func_801DAED4(void);
void submitPanelPair(s16 arg0, s16 arg1);
void configureSpriteClut(s16 arg0, s16 arg1, u8 arg2);
void appendDimTile(void);

#define WORLD00_AREA030_SCRATCH_PTR PSX_REF(volatile u8*, 0x1f800044u)

#endif
