#ifndef EMI_WORLD00_AREA016_13_INTERNAL_H
#define EMI_WORLD00_AREA016_13_INTERNAL_H

#include "bof3/bof3.h"

typedef void (*World00Area016Handler)(void);

typedef struct World00Area016MarkerEntry {
  u16 mask;
  u8  field_02;
  u8  field_03;
} World00Area016MarkerEntry;

/* Front-end record table at 0x80143FC8 — 0x74 bytes per record. Fields proven
 * for this target: the occupancy byte at 0x00, the byte at 0x01, the selector
 * byte at 0x05, the byte at 0x06, the byte at 0x08 and the signed timer at
 * 0x3E. */
typedef struct World00Area016Record {
  u8  flags_00;
  u8  unk_01;
  u8  unk_02;
  u8  unk_03;
  u8  unk_04;
  u8  unk_05;
  u8  unk_06;
  u8  unk_07;
  u8  unk_08;
  u8  unk_09[0x35]; /* 0x09 - 0x3D */
  s16 timer_3E;     /* 0x3E */
  u8  unk_40[0x34]; /* 0x40 - 0x73 */
} World00Area016Record;

typedef struct World00Area016Scratch {
  u8  unk_00;
  u8  mode;
  u8  state_02;
  u8  state_03;
  u8  unk_04[0x07];
  u8  unk_0b;
  u32 unk_0c;
  u32 unk_10;
  u8  unk_14[0x04];
  u32 unk_18;
  u8  unk_1c[0x12];
  s16 field_2e;
  s16 field_30;
  u8  unk_32[0x02];
  u32 unk_34;
  u32 unk_38;
  u32 unk_3c;
  u32 unk_40;
  u32 unk_44;
  u8  unk_48;
} World00Area016Scratch;

/* @source 0x1F800014 @kind unknown */
extern SVECTOR D_1F800014[];
/* @source 0x1F800044 @kind unknown */
extern volatile World00Area016Scratch* D_1F800044;
/* @source 0x801F4D7C @kind unknown */
extern const World00Area016MarkerEntry D_801F4D7C[];
/* @source 0x801F4DA0 @kind unknown */
extern const u8 D_801F4DA0[][4];
/* @source 0x801F4FC4 @kind unknown */
extern u16 D_801F4FC4[];
/* @source 0x801F50E4 @kind unknown */
extern u8 D_801F50E4[];
/* @source 0x801F50E5 @kind unknown */
extern u8 D_801F50E5[];
/* @source 0x801F50F4 @kind unknown */
extern u8 D_801F50F4[];
/* @source 0x801F5194 @kind unknown — 4-byte marker layers (mask at 0x00,
 * code at 0x02) walked by func_801F3B00. Declared as an extern array so `as`
 * expands the indexed relocation with the original `addu $at,$at,$idx` operand
 * order, as for the other local record tables. */
extern const World00Area016MarkerEntry D_801F5194[];
/* @source 0x801F5100 @kind unknown */
extern World00Area016Handler D_801F5100[];
/* @source 0x801F5114 @kind unknown */
extern World00Area016Handler D_801F5114[];
/* @source 0x801F511C @kind unknown */
extern World00Area016Handler D_801F511C[];
/* @source 0x801F512C @kind unknown */
extern World00Area016Handler D_801F512C[];
/* @source 0x801F51D0 @kind unknown */
extern World00Area016Handler D_801F51D0[];
/* @source 0x801F51AC @kind unknown */
extern World00Area016Handler D_801F51AC[];
/* @source 0x80104000 @kind unknown */
extern u8 D_80104000;
/* @source 0x80104001 @kind unknown — resident AREA016 map height (the width is
 * D_80104000; the same pair is evidenced by `emi/etc/game/00`). */
extern u8 D_80104001;
/* @source 0x80143B90 @kind unknown */
extern u16 D_80143B90;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80143E6C @kind unknown */
extern u32 D_80143E6C;
/* @source 0x80143F10 @kind unknown */
extern u16 D_80143F10;
/* @source 0x80143FC8 @kind bss */
extern World00Area016Record D_80143FC8[];
/* @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC[];
/* @source 0x801448ED @kind unknown */
extern u8 D_801448ED;
/* @source 0x8014496D @kind unknown */
extern u8 D_8014496D;
/* @source 0x80144FE4 @kind unknown */
extern u8 D_80144FE4[];
/* @source 0x801454F2 @kind unknown */
extern u8 D_801454F2;
/* @source 0x80145E99 @kind unknown */
extern u8 D_80145E99;
/* @source 0x80145E9C @kind unknown */
extern s32 D_80145E9C;
/* @source 0x80145EA0 @kind unknown */
extern s32 D_80145EA0;
/* @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC4;
/* @source 0x80145EC6 @kind unknown */
extern s16 D_80145EC6[];
/* @source 0x80145EC8 @kind unknown */
extern s32 D_80145EC8;
/* @source 0x80145ECA @kind unknown */
extern s16 D_80145ECA;
/* @source 0x80145AB4 @kind unknown */
extern u16 D_80145AB4;
/* @source 0x80145AC0 @kind unknown */
extern u16 D_80145AC0;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x8014625A @kind unknown */
extern u16 D_8014625A;
/* @source 0x80146870 @kind unknown */
extern s8 D_80146870;
/* @source 0x80146871 @kind unknown */
extern u8 D_80146871;
/* @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;
/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801490D8 @kind unknown */
extern u8 D_801490D8[];
/* @source 0x801490E4 @kind unknown */
extern u8 D_801490E4[];
/* @source 0x8014930A @kind unknown */
extern s16 D_8014930A;
/* @source 0x8014930E @kind unknown */
extern s16 D_8014930E;
/* @source 0x8014931C @kind unknown */
extern u32 D_8014931C;
/* @source 0x8014832E @kind unknown */
extern u8 D_8014832E;
/* @source 0x80010008 @kind unknown */
extern u16 D_80010008;
/* @source 0x8014598C @kind unknown */
extern u8* g_PrimCursor;
/* @source 0x801F513C @kind unknown */
extern volatile u8 D_801F513C[];
/* @source 0x801F51B8 @kind unknown */
extern const u16 D_801F51B8[][2];
/* @source 0x801F51C8 @kind unknown */
extern const u8 D_801F51C8[][2];
/* Target-local texture-atlas descriptor columns: one byte per panel layer,
 * stored as three columns four bytes apart. D_801F51D8 is the u origin,
 * D_801F51DC the v origin and D_801F51E0 the square tile size. */
/* @source 0x801F51D8 @kind unknown */
extern u8 D_801F51D8[];
/* @source 0x801F51DC @kind unknown */
extern u8 D_801F51DC[];
/* @source 0x801F51E0 @kind unknown */
extern u8 D_801F51E0[];

void func_8014D260(void);
void func_8014D290(void);
void func_8014D4E0(void);
void func_8014D6B8(u32 flag);
u8   func_8014D978(void);
void func_8014DD3C(s32 arg0);
void func_8014E5A0(u8 arg0, u8 arg1);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
s32  func_80166CB0(s16 arg0, s16 arg1);
void func_80150224(s32 arg0);
void func_80155560(u32 arg0, void* arg1, s32 arg2);
void* func_801559AC(s32 arg0, s32 arg1);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8015C058(void);
void func_8015C088(void);
u8*  func_80165D48(s32 arg0, u8 arg1);
u8   func_801B6610(s16 arg0, s16 arg1);
void func_80196070(void);
u8   func_8019601C(void);
s16  func_8015477C(s32 arg0, s32 arg1);
u8   func_8019A194(void);

void func_801F2C04(void);
void seedScratchDefaults(void);
void dispatchState02(void);
void advanceScratchHeightState02(void);
void advanceState02Step(void);
void func_801F3608(void);
void dispatchState03(void);
void func_801F36D0(void);
void func_801F3750(void);
void func_801F3828(void);
void func_801F3928(void);
void emitSemiTransparentSprite(s16 arg0, s16 arg1, u8 arg2);
void func_801F3B00(s16 arg0, s16 arg1);
void appendTransformedG4Panel(s16 arg0, s16 arg1);
void func_801F40C4(s16 arg0, s16 arg1);
void dispatchArea016ScratchMode(void);
void func_801F45CC(void);

#define WORLD00_AREA016_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area016Scratch*, 0x1f800044u)
#define WORLD00_AREA016_PRIMITIVE_PTR PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA016_SPRT_TABLE PSX_PTR(const volatile u8, 0x801f513cu)
#define WORLD00_AREA016_ROTATION   PSX_PTR(SVECTOR, 0x801492d8u)
#define WORLD00_AREA016_G4_VERTEX0 SPAD_ADDR(SVECTOR, 0x14u)
#define WORLD00_AREA016_G4_VERTEX1 SPAD_ADDR(SVECTOR, 0x1cu)
#define WORLD00_AREA016_G4_VERTEX2 SPAD_ADDR(SVECTOR, 0x24u)
#define WORLD00_AREA016_G4_VERTEX3 SPAD_ADDR(SVECTOR, 0x2cu)

#endif
