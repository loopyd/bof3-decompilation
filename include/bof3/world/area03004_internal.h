#ifndef EMI_WORLD00_AREA030_04_INTERNAL_H
#define EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <rand.h>

/* Main-executable VRAM halfword pair selected by bit 0 of the shared AREA030
 * panel flag byte 0x80144287: the flagged handler at 0x801DCCF4 submits the
 * chosen halfword offset by +0x80010000 to func_8014F800. */
extern u16 D_800100A2;    /* @source 0x800100A2 @kind unknown */
extern u16 D_800100A4;    /* @source 0x800100A4 @kind unknown */
/* Shared mode byte; stored small mode values (4/6/7/...) and compared
 * against them by the area030 mode handlers. */
extern u16 D_80143C40;    /* @source 0x80143C40 @kind unknown */
extern s32 D_80143E6C;    /* shared frame counter */
extern u16 D_80143B92;    /* @source 0x80143B92 @kind unknown */
extern u8  D_8014832E;    /* @source 0x8014832E @kind unknown */
extern u8  D_80149332;    /* @source 0x80149332 @kind unknown */
extern u16 D_8014932E;    /* @source 0x8014932E @kind unknown */
extern u32 D_8014930C;    /* @source 0x8014930C @kind unknown */
extern s16 D_8014930E;    /* @source 0x8014930E @kind unknown */
extern u8  D_80143FC9;    /* @source 0x80143FC9 @kind unknown */
/* Shared front-end entry table at 0x80143FC8: 20 records of 0x74 bytes each
 * (the recordTable whose whole 0-4 byte head clearRecord zeroes). func_801E0A30
 * walks the head bytes 1-4 of the leading slots through this 0x74-byte view, so
 * the table base sits one byte before the D_80143FC9 scalar view above. */
typedef struct Area030EntryRecord {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 pad_05[0x6F];
} Area030EntryRecord;
extern Area030EntryRecord D_80143FC8[]; /* @source 0x80143FC8 @kind bss */
/* Shared CLUT-bank byte read as the trailing argument of the main-executable
 * shaded-panel helper func_801AE3F0 by the AREA030 cell drawers. */
extern u8  D_80144952;    /* @source 0x80144952 @kind unknown */
/* Shared palette-stage serial byte published as 1 by the AREA030 color-row
 * builder func_801D130C after it rewrites the 0x800396C0 color row. */
extern u8  D_80145988;    /* @source 0x80145988 @kind unknown */
/* Signed limit byte (and its +3 companion byte) of the AREA030 message state
 * advanced by func_801DE018. */
extern s8  D_80143FCF;    /* @source 0x80143FCF @kind unknown */
/* Front-end step word and its +4 companion word: func_801DA78C steps
 * D_80143FD8 back by 0x4000 while D_80143FDC stays clear, and reads the
 * companion as a plain scalar at its own address. */
extern s32 D_80143FD8;    /* @source 0x80143FD8 @kind unknown */
extern s32 D_80143FDC;    /* @source 0x80143FDC @kind unknown */
extern s32 D_80143FFC;    /* @source 0x80143FFC @kind unknown */
extern s32 D_80144000;    /* @source 0x80144000 @kind unknown */
extern s16 D_80144006;    /* @source 0x80144006 @kind unknown */
extern u8  D_8014403D;    /* @source 0x8014403D @kind unknown */
/* AREA030 step counter byte at the companion address of D_8014403D. The
 * 0x801DA92C original translation unit addresses it through its materialized
 * base register (array view), unlike the folded scalar views above. */
extern u8  D_8014403E[];  /* @source 0x8014403E @kind unknown */
extern u8  D_801440B1;    /* @source 0x801440B1 @kind unknown */
extern u8  D_80144FE4[];  /* @source 0x80144FE4 @kind unknown */
extern u8  D_80144281;    /* @source 0x80144281 @kind unknown */
extern u8  modeByte;      /* @source 0x80144125 @kind bss */
extern u8  D_8014412A;    /* @source 0x8014412A @kind unknown */
extern u8  D_8014412B;    /* @source 0x8014412B @kind unknown */
extern u8  D_80144199[];  /* @source 0x80144199 @kind unknown */
/* @source 0x80144199 @kind unknown: byte-scalar view of the shared AREA030
 * selection gate. The 0x801DAF18 original translation unit reads and increments
 * this byte as a plain scalar, folding each access at the symbol address, while
 * the D_80144199 array view materializes its base into a register. */
extern u8  D_80144199_BYTE;
extern u8  D_8014419E;    /* @source 0x8014419E @kind unknown */
extern u8  D_80144286;    /* @source 0x80144286 @kind unknown */
/* Shared AREA030 panel flag byte: the 0x801DCCF4 handler masks it to seven
 * bits and publishes the masked value back, and selects its main-executable
 * VRAM halfword through its bit 0. */
extern u8  D_80144287;    /* @source 0x80144287 @kind unknown */
extern u8  D_80145E92;    /* @source 0x80145E92 @kind unknown */
extern u8  D_80145E93;    /* @source 0x80145E93 @kind unknown */
/* Shared AREA030 color-block row index byte: func_801D130C reads it once per
 * copied element and uses it to select the 0x40-byte row of the 0x80037800
 * color block that feeds the 0x800396C0 color row. */
extern u8  D_80145EB7;    /* @source 0x80145EB7 @kind unknown */
/* Shared AREA030 panel-template bytes published into the scratch work record
 * by func_801D11C0 (offsets 0x2A and 0x49..0x4B) and its shared word/halfword
 * companions at 0x80145EE0/0x80145EE4 (scratch 0x50/0x54) and
 * 0x80145EE8/0x80145EEA (scratch 0x58/0x5A). */
extern u8  D_80145EBA;    /* @source 0x80145EBA @kind unknown */
extern u8  D_80145ED9;    /* @source 0x80145ED9 @kind unknown */
extern u8  D_80145EDA;    /* @source 0x80145EDA @kind unknown */
extern u8  D_80145EDB;    /* @source 0x80145EDB @kind unknown */
extern s32 D_80145EE0;    /* @source 0x80145EE0 @kind unknown */
extern s32 D_80145EE4;    /* @source 0x80145EE4 @kind unknown */
extern u16 D_80145EE8;    /* @source 0x80145EE8 @kind unknown */
extern u16 D_80145EEA;    /* @source 0x80145EEA @kind unknown */
/* Shared pointer cell owned by the main executable; the AREA030 step
 * initializer func_801D9D38 gates its resource request on byte 0x79 of the
 * record it points at. */
extern u8* D_80146250;    /* @source 0x80146250 @kind unknown */
extern void (*D_801E2000[])(void); /* @source 0x801E2000 @kind table */
extern void (*D_801E2014[])(void); /* @source 0x801E2014 @kind table */
extern void (*D_801E2048[])(void); /* @source 0x801E2048 @kind table */
extern void (*D_801E2074[])(void); /* @source 0x801E2074 @kind table */
/* Four-byte AREA030 step records at 0x801E207C walked by func_801D6CB0 through
 * the work-record step byte (offset 10 of the record published at the scratchpad
 * cursor 0x1F800044): byte 0 is the 0xFF end marker and the 0x80/0x7F request
 * bits, byte 1 the step countdown copied into work byte 9, byte 2 the delta
 * integrated into work word 0x0C, and byte 3 the delta integrated into work word
 * 0x10 whose sign selects the func_801D6B28 marker mode. The byte-0 and byte-2
 * request and subtraction sites read these bytes unsigned, so they reach the
 * table through an address-derived byte view of this same symbol. */
typedef struct Area030StepRecord {
  s8 unk_00;   /* +0x00: 0xFF end marker; bit 7 gates the func_8014D978 call */
  u8 count_01; /* +0x01: step countdown published into work byte 9 */
  s8 unk_02;   /* +0x02: delta added to work word 0x0C and work byte 6 */
  s8 unk_03;   /* +0x03: delta added to work word 0x10 */
} Area030StepRecord;
/* Unsigned byte view of the same 4-byte step records: the request byte masked
 * into the func_8014D8D4 argument and the second delta subtracted from work byte
 * 6 are read with an unsigned byte load (`lbu`) while the end-marker, sign and
 * integration sites read the signed fields above. */
typedef struct Area030StepRecordBytes {
  u8 unk_00;
  u8 count_01;
  u8 unk_02;
  u8 unk_03;
} Area030StepRecordBytes;
extern Area030StepRecord D_801E207C[]; /* @source 0x801E207C @kind table */
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
/* Per-index halfword limits compared by func_801E01DC with the work-record
 * halfword at +0x58, selected by the record byte at +0x06. */
extern u16 D_801E2354[];  /* @source 0x801E2354 @kind table */
extern void (*D_801E22E4[])(void); /* @source 0x801E22E4 @kind table */
extern u8  D_801D0C30[];  /* @source 0x801D0C30 @kind rodata */
extern u8  D_801D0C38[];  /* @source 0x801D0C38 @kind rodata */
typedef struct IconRecord {
  u8 icon, offset, first, second;
} IconRecord;
typedef struct IconData {
  IconRecord records[13];
} IconData;
typedef struct ThresholdData {
  u16 thresholds[13];
} ThresholdData;
extern IconData D_801D0C3C; /* @source 0x801D0C3C @kind rodata */
extern ThresholdData D_801D0C70; /* @source 0x801D0C70 @kind rodata */
extern u8 D_801D0C8C[]; /* @source 0x801D0C8C @kind rodata */
extern u8  D_801C8964[];  /* @source 0x801C8964 @kind unknown */
/* Three-entry AREA030 work-record stage-handler table at 0x801E1D00, indexed
 * by the record stage byte at scratch offset 3 by func_801D12C8. The three
 * code pointers observed at 0x801E1D00/0x801E1D04/0x801E1D08 are the
 * consecutive stage handlers 0x801D130C, 0x801D1434 and 0x801D1538. It sits
 * inside the T_801E1CD4 data blob, preceded there by an unrelated pointer
 * block, so the table base is the address the original code indexes from. */
extern void (*D_801E1D00[])(void); /* @source 0x801E1D00 @kind table */
/* Four-byte AREA030 texture-cell u0 table (bytes C0 D0 E0 D0) indexed by bits
 * 3-4 of the shared frame counter D_80143E6C by func_801D1818. */
extern u8 D_801E1D0C[]; /* @source 0x801E1D0C @kind table */
extern u8  D_801E28C4[];  /* @source 0x801E28C4 @kind rodata */
extern u8  D_801E28D0[];  /* @source 0x801E28D0 @kind rodata */
/* Four 8-byte AREA030 text slot records read by func_801DE018: a text pointer
 * followed by the slot index byte. D_801E28F8 is an s8 byte-offset view of the
 * same index bytes (8-byte stride), which the original walks with its own
 * offset register. */
typedef struct Area030TextSlot {
  u8* text;
  s8  index;
  u8  pad_05[3];
} Area030TextSlot;
extern Area030TextSlot D_801E28F4[]; /* @source 0x801E28F4 @kind table */
extern s8 D_801E28F8[];              /* @source 0x801E28F8 @kind table */
extern u8  D_80145AD4[];  /* @source 0x80145AD4 @kind unknown */
extern u16 D_801E218C[]; /* @source 0x801E218C @kind table */
extern u16 D_801E218E[]; /* @source 0x801E218E @kind table */
extern u16 D_801E2190[]; /* @source 0x801E2190 @kind table */
extern u8 D_801E2192[]; /* @source 0x801E2192 @kind table */
extern u16 D_801E2194[]; /* @source 0x801E2194 @kind table */
extern u8  D_801E2384[];  /* @source 0x801E2384 @kind unknown */
extern u8  D_801E2388[];  /* @source 0x801E2388 @kind unknown */
extern u8  D_801E238C[];  /* @source 0x801E238C @kind unknown */
extern u8  D_801E2390[];  /* @source 0x801E2390 @kind unknown */
extern u8* g_PrimCursor;    /* @source 0x8014598C @kind unknown */
extern s32 D_8014421C;     /* @source 0x8014421C @kind unknown */
extern u16 D_80145AA8;     /* @source 0x80145AA8 @kind unknown */
extern u16 D_80145AC2;     /* @source 0x80145AC2 @kind unknown */
extern u16 D_80145AC4;     /* @source 0x80145AC4 @kind unknown */
extern u16 D_80145AA4;     /* @source 0x80145AA4 @kind unknown */
extern u8  D_8014504C[];   /* @source 0x8014504C @kind unknown */
extern u8  D_8014524C[];   /* @source 0x8014524C @kind unknown */
extern u8  D_801E31F0;     /* @source 0x801E31F0 @kind unknown */
extern u8  D_801E31F4;     /* @source 0x801E31F4 @kind unknown */
extern u8  D_801E31F8;     /* @source 0x801E31F8 @kind unknown */
/* Companion byte of the 0x801E31F8 selected-record value: the byte holding the
 * record value whose cue was played last, read by func_801DDD4C as a plain
 * scalar at its own address. */
extern u8  D_801E31FC;     /* @source 0x801E31FC @kind unknown */
/* AREA030 text slot tables walked by func_801DE018: D_801E31E0 holds the four
 * per-slot character counters and D_801E31E4 the four per-slot frame timers. */
extern u8  D_801E31E0[];   /* @source 0x801E31E0 @kind unknown */
extern u8  D_801E31E4[];   /* @source 0x801E31E4 @kind unknown */
/* Nonzero byte flag at the end of this overlay's data; read by
 * func_801E1C7C to gate its extra icon and cleared by func_801DBBAC while
 * the shared pad mask 0x80145AA8/0x50 is set. */
extern u8  D_801E3214;     /* @source 0x801E3214 @kind unknown */
extern u8 D_80145026;      /* @source 0x80145026 @kind unknown */
/* Companion byte of the 0x80145026 AREA030 level flag: cleared together with
 * the flag by func_801DC4B8 when the frontend slot setter reports the flag
 * cannot be registered, and read as a plain scalar at its own address. */
extern u8 D_80145027;      /* @source 0x80145027 @kind unknown */
extern u8 D_80145028;      /* @source 0x80145028 @kind unknown */
extern u8 D_801E2720[];    /* @source 0x801E2720 @kind unknown */
extern u8 D_801E2748[];    /* @source 0x801E2748 @kind unknown */
/* Flag byte armed/cleared by the 0x40 mode handler func_801D9E40: set when the
 * func_801E1C10 icon row is full, cleared otherwise. */
extern u8  D_801E3204;      /* @source 0x801E3204 @kind unknown */
extern u8  D_801E3208;      /* @source 0x801E3208 @kind unknown */
extern s8* D_801E320C;     /* @source 0x801E320C @kind unknown */
extern u8* D_801E3210;     /* @source 0x801E3210 @kind unknown */

/* Global route/step table pair owned by the main executable: route entry `i`
 * is the step pair D_80181AC0[i * 2] / D_80181AC4[i * 2]. func_801E03F8
 * reads one entry through the work-record route byte and scans the eight
 * entries again to re-index it. */
extern s32 D_80181AC0[];  /* @source 0x80181AC0 @kind table */
extern s32 D_80181AC4[];  /* @source 0x80181AC4 @kind table */

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

/* Shared 0x98-byte work records at 0x80146888 (0x1E slots). The AREA030
 * handlers walk them, publish the active record as the scratchpad cursor at
 * 0x1F800044 and as D_80146884, then dispatch the record's handler byte
 * through the overlay-local handler table. */
typedef struct Area030WorkRecord {
  u8  flags_00;      /* +0x00: record flags; 0x20 gates the level update */
  u8  handler_01;    /* +0x01: handler index into D_801E2314 */
  u8  unk_02[0x32];  /* +0x02-0x33 */
  s32 x_34;          /* +0x34: signed record x the area handlers compare */
  s32 y_38;          /* +0x38: signed record y the area handlers compare */
  u8  unk_3C[2];     /* +0x3C-0x3D */
  s16 counter_3E;    /* +0x3E: signed countdown the level derives from */
  u8  unk_40[0x1D];  /* +0x40-0x5C */
  s8  level_5D;      /* +0x5D: level derived from counter_3E */
  u8  unk_5E;        /* +0x5E: level mirror */
  u8  unk_5F;        /* +0x5F: level mirror */
  u8  unk_60[0x38];  /* +0x60-0x97 */
} Area030WorkRecord;

extern Area030WorkRecord D_80146888[]; /* @source 0x80146888 @kind unknown */
extern void (*D_801E2314[])(void); /* @source 0x801E2314 @kind table */

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

/* AREA030 per-slot scale records: func_801DF334 forms
 * `0x801E2AB8 + index * 36` and the sibling slot handlers read words at
 * +0x10/+0x12 and bytes at +0x1A..+0x20 of the same records. Only the two
 * fields read by func_801E0ABC are named here. */
typedef struct Area030ScaleRecord {
  u8 unk_00[0x1B];
  u8 unk_1B;     /* +0x1B: single-byte scale row read by func_801E03F8 */
  u8 unk_1C[3];
  u8 limit;      /* +0x1F: highest slot amount that still scales */
  u8 unk_20[2];
  u16 value;     /* +0x22: value reported at the full limit */
} Area030ScaleRecord;

extern Area030ScaleRecord D_801E2AB8[]; /* @source 0x801E2AB8 @kind table */

/* Scratchpad work-record cursor cell; reloaded per store group by the
 * AREA030 handlers (volatile cell, plain RAM pointee). */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind bss */

/* Scoped companion-call ABI for func_801E0C20; not a callback contract. */
void dispatchArea030CompanionHandler(void); /* @source 0x800F500C */
void func_8014D290(void);
u8   func_8014D978(void);
/* Same-binary frontend slot setter at 0x801665A0: the AREA030 call sites pass
 * (slot 3, a byte value, 1, 0) and either discard the result or test only its
 * low byte, so the AREA030 view declares a four-argument byte-valued helper. */
u8   func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Target-local pad-mask gate at 0x801DDF7C: its `and` of D_80145AA8 with the
 * caller's argument selects the gate path and it returns nonzero on the taken
 * path (frontend mode 2 plus scratch bytes 2/3). Only the argument matters to
 * the AREA030 call site, which discards the gate result. */
u32  func_801DDF7C(u16 mask);
void func_8014D4E0(void);
u32  func_8014D8D4(u8 arg0);
void func_8014DD3C(s32 arg0);
/* Shared resource-request helper at 0x8014D6B8: the AREA030 record initializer
 * func_801D6414 calls it with the single flag 3 (direct jal at 0x801D64B8). */
void func_8014D6B8(s32 arg0);
/* Shared resource-request helper at 0x8014D6DC: the AREA030 step initializer
 * func_801D9D38 calls it with (0xA, 2) when the flag byte at D_80146250 + 0x79
 * is clear. The shipped executable body reads the scratchpad cursor at
 * 0x1F800044 and masks its first argument with 0x80, confirming the
 * two-argument form (direct jal at 0x801D9DC8). */
void func_8014D6DC(s32 arg0, s32 arg1);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
int  func_8017E3F4(char* buffer, char* fmt, ...);
void func_801D195C(s16 arg0, s16 arg1);
void func_801D18CC(s16 arg0, s16 arg1, u8 arg2);
/* Target-local icon-row/numeric-label drawer at 0x801E1320: the AREA030 panel
 * step calls it with the constant top-left corner (176, 156). */
void func_801E1320(s16 x, s16 y);
/* Target-local sprite/texture draw at 0x801E162C: the AREA030 panel step calls
 * it with (8, 148, 66). */
void func_801E162C(s32 x, s32 y, u16 image);
void submitTpageDrawMode(s32 arg0, s32 arg1);
u32 func_801E0B6C(void);
extern u8 D_801454EC; /* @source 0x801454EC @kind unknown */
u8*  func_801E0DCC(s32 arg0, s32 arg1, s16 arg2, s16 arg3);
void func_801D799C(s16 arg0, s16 arg1, u8 arg2, u8 arg3);
void func_801D934C(u16 arg0, u16 arg1, u16 arg2, u16 arg3);
void func_801D95C4(u16 arg0, u16 arg1, u16 arg2, u16 arg3, u8 arg4);
void func_801D9534(s16 arg0, u16 arg1, s16 arg2, s16 arg3, u8 arg4);
/* Target-local AREA030 panel drawer at 0x801E1758: the AREA030 panel steps call
 * it with the computed vertical origin and the constant 0x56. */
void func_801E1758(s16 arg0, s16 arg1);
void func_80196070(void);
/* Same-binary entry-table clearer at 0x801960C0 (clearRecord in the game00
 * view): clears the five head bytes of one 0x74-byte record slot. The AREA030
 * reset passes a slot index, which the u8 parameter truncates. */
void clearRecord(u8 record_index);
/* Same-binary frontend selection-effect starter at 0x8015D4F8 (executable
 * space, called by the AREA030 selection step with (0, 0, 100, 8)). */
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
/* Same-binary sound-cue dispatcher at 0x8015DF18 (executable space): the
 * AREA030 panel step calls it with the fixed cue 0x102. */
void func_8015DF18(u16 arg0);
/* Same-binary shaded-panel rectangle helper at 0x801AE3F0 (executable space):
 * the AREA030 cell drawer submits its 0x28 by 0x10 panel rectangle at the
 * two-pixel inset corner of the requested origin, with the shared CLUT-bank
 * byte D_80144952 as the trailing argument. Body not lifted in this target. */
void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                   u32 arg5);
/* Target-local prototype for the co-resident front-end entry probe at
 * 0x801BDD58 (direct `jal` at 0x801E0738 in func_801E0680): it takes the
 * record x at +0x34, y at +0x38 and signed timer at +0x3E, a byte range and
 * the entry-table pointer 0x80143FC8 in its fifth stack argument, and reports
 * a byte-valued match. Body not lifted in this target. */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range, void* work);
s16  func_8015477C(u16 arg0, u16 arg1);
s32  func_801E0ABC(s32 arg0, s32 arg1);
void func_801E19CC(s16 arg0, s16 arg1);
s32  func_801E1C10(void);
void func_801E084C(void); /* per-record scroll updater on the 0x1F800044 cursor */
/* Same-binary shared work-area selector at 0x80199440 (executable space, direct
 * `jal` at 0x801E0BD8): publishes one of the three shared work-area bases into
 * D_80146250 and the scratchpad cursor according to the load flags and takes no
 * arguments. Body not lifted in this target. */
void func_80199440(void);
/* Same-binary frame-service entries (executable space) run in this order by the
 * AREA030 frame chain functions updateArea030OverlayFrame (jals at 0x801E0BE8,
 * 0x801E0BF0, 0x801E0BF8, 0x801E0C00, 0x801E0C08) and runArea030FrameServices
 * (jals at 0x801E0C48, 0x801E0C50, 0x801E0C58, 0x801E0C60, 0x801E0C68). The
 * game00 view declares them void(void). Bodies not lifted in this target. */
void func_801527E4(void);
void func_801BDAB8(void);
void func_8019A0E4(void);
void func_8014BA54(void);
/* Target-local 0x1E-record dispatcher at 0x801DE4AC: publishes every active
 * shared work record at the scratchpad cursor 0x1F800044 and dispatches its
 * handler byte through the overlay handler table D_801E2314. */
void func_801DE4AC(void);
/* Target-local per-record scroll updater loop at 0x801E09B4. */
void func_801E09B4(void);
/* AREA030 overlay per-frame entry at 0x801E0BD0: invoked once by the
 * main-executable front-end substate-4 handler func_801984E8 (jal 0x80198578).
 */
void updateArea030OverlayFrame(void);
/* Shared frame-service chain at 0x801E0C40: invoked by the front-end substate-4
 * wait loops func_801985C8 (jals 0x80198614, 0x80198694). */
void runArea030FrameServices(void);

void func_801D11C0(void);
void func_801D12C8(void);
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
/* @source 0x8017AA08 — shared libgpu 16x16 sprite setter (three-word packet,
 * code 0x7C) verified in the shipped executable; absent from the SDK map, so it
 * stays target-local. */
void func_8017AA08(SPRT_16* arg0);
void func_801D1B88(s16 arg0, s16 arg1, s16 arg2, u8 arg3);
void queueIconStrip(s16 arg0, s16 arg1, u8 arg2, s8 arg3);
void func_801D2AE0(void);
void func_801D2C34(s16 arg0, s16 arg1, s8 arg2, u8 arg3);
void func_801D3244(s16 arg0, s16 arg1, u8 arg2, s8 arg3, u8 arg4, s8 arg5);
void advancePanelScroll(void);
void func_801D6F08(void);
void seedMenuScratch(void);
void func_801D6B28(s8 arg0);
void func_801D6CB0(void);
void func_801D6EC4(void);
void advanceStepMode6(void);
void clearFlagState10(void);
void func_801E026C(s8 arg0);
void func_801DAE3C(void);
void dispatchCompanion(void);
void func_801DFFA8(void);
void func_801DCC74(void);
void func_801DCCB0(void);
void func_801DCCF4(void); /* AREA030 handler-table entry 0 at 0x801E22F0 */
/* Same-binary panel position submitter at 0x801647C4 (executable space): the
 * AREA030 flagged handler passes the panel id 0x1C and the masked flag byte
 * scaled by 16 plus 0x58 as an unsigned halfword pair, and discards the result. */
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
/* Same-binary shaded-panel helper at 0x8014F800 (executable space), called by
 * the AREA030 flagged handler with (0x1D, 0x14, 0, 0xFF) and the selected
 * main-executable halfword offset by the 0x80010000 VRAM base. */
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
void func_801DDFD4(void);
void func_801DDE94(s32 state);
void func_801DDDBC(void);
void func_801DC474(void);
void func_801DC590(void);
void func_801DC64C(void);
void func_801DC708(void);
void func_801DC7EC(void);
void func_801DA5F8(void);
void func_801DA8E8(void);
void func_801DAE84(void);
void func_801DAED4(void);
/* Target-local panel decoration/label drawer at 0x801E1004: the AREA030 panel
 * steps pass the active record's byte 6 as the selected character, an unused
 * zero, and the remaining count. */
void func_801E1004(s32 arg0, s32 arg1, s32 arg2);
/* Target-local extra-icon drawer at 0x801E1C7C, gated by the D_801E3214 flag
 * byte and bit 3 of the shared frame counter D_80143E6C. */
void func_801E1C7C(void);
void submitPanelPair(s16 arg0, s16 arg1);
void configureSpriteClut(s16 arg0, s16 arg1, u8 arg2);
void appendDimTile(void);

#define WORLD00_AREA030_SCRATCH_PTR PSX_REF(volatile u8*, 0x1f800044u)

#endif
