#ifndef EMI_WORLD00_AREA030_04_INTERNAL_H
#define EMI_WORLD00_AREA030_04_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <rand.h>

/* Main-executable halfword text offset pair selected by bit 0 of the shared AREA030
 * panel flag byte 0x80144287: the flagged handler at 0x801DCCF4 submits the
 * chosen halfword offset by +0x80010000 to drawEncodedTextAt. */
extern u16 D_800100A2;    /* @source 0x800100A2 @kind unknown */
extern u16 D_800100A4;    /* @source 0x800100A4 @kind unknown */
/* Main-executable halfword text offset read per call by the AREA030 handler at
 * 0x801DD7CC and submitted to drawEncodedTextAt offset by the same +0x80010000
 * base: (0x1D, 0x14, 0, 0xFF, base + halfword). */
extern u16 D_800100E0;    /* @source 0x800100E0 @kind unknown */
/* Main-executable halfword text offset read per call by the AREA030 pad handler at
 * 0x801DD664 and submitted to drawEncodedTextAt offset by the same +0x80010000
 * base: (0x1D, 0x14, 0, 0xFF, base + halfword). */
extern u16 D_800100C2;    /* @source 0x800100C2 @kind unknown */
/* Unsigned halfword text offsets used when work byte 3 is 2. Masked handler
 * selectors 0, 1 and 2 choose 0x800100A0, 0x800100C0 or 0x800100DE.
 * The step at 0x801DC93C adds 0x80010000 and submits the selected byte stream
 * to drawEncodedTextAt. Live contents and resource ownership remain unresolved. */
extern u16 handlerSelectorZeroTextOffsetState;    /* @source 0x800100A0 @kind unknown */
extern u16 handlerSelectorOneTextOffsetState;    /* @source 0x800100C0 @kind unknown */
extern u16 handlerSelectorTwoTextOffsetState;    /* @source 0x800100DE @kind unknown */
/* Shared main-executable equipment record table: the AREA030 name steps scale
 * the selected equipment index by 20 bytes, the serialized record stride whose
 * leading 0x0C bytes are the record name the shared text drawer prints. */
extern u8  D_801C9E7C[];  /* @source 0x801C9E7C @kind table */
/* Shared mode byte; stored small mode values (4/6/7/...) and compared
 * against them by the area030 mode handlers. */
extern u16 D_80143C40;    /* @source 0x80143C40 @kind unknown */
extern s32 D_80143E6C;    /* shared frame counter */
extern u16 D_80143B92;    /* @source 0x80143B92 @kind unknown */
extern u8  D_8014832E;    /* @source 0x8014832E @kind unknown */
extern u8  D_80149332;    /* @source 0x80149332 @kind unknown */
extern u16 D_8014932E;    /* @source 0x8014932E @kind unknown */
/* Shared fixed-point channel pair committed together by the AREA030 dispatch
 * step 0x801DC74C and by the scenario setup steps: the shipped executable reads
 * the pair as the origin it subtracts from the work record's +0x34/+0x38 words
 * before the 20.12 scaling. */
extern u32 D_80149308;    /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C;    /* @source 0x8014930C @kind unknown */
extern s16 D_8014930E;    /* @source 0x8014930E @kind unknown */
extern u8  D_80143FC9;    /* @source 0x80143FC9 @kind unknown */
/* Shared front-end entry table at 0x80143FC8: 20 records of 0x74 bytes each
 * (the recordTable whose whole 0-4 byte head clearRecord zeroes). clearRecordHeadFieldsAndSetStateTwoWorld00Area03004
 * walks the head bytes 1-4 of the leading slots through this 0x74-byte view, so
 * the table base sits one byte before the D_80143FC9 scalar view above. */
typedef struct Area030EntryRecord {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 byte_05;
  u8 byte_06;
  u8 pad_07[4];
  u8 byte_0B;
  u8 pad_0C[0x22];
  /* Halfword pair the work-byte-3 handler index 1 at 0x801D6128 stores into the
   * record it arms for the 0x50 step mode: 0xF0 at +0x2E and 0xA6 at +0x30. */
  u16 halfword_2E;
  u16 halfword_30;
  u8 pad_32[0x18];
  u8 byte_4A;
  u8 pad_4B[0x29];
} Area030EntryRecord;
ASSERT_SIZE(Area030EntryRecord, 0x74);
extern Area030EntryRecord D_80143FC8[]; /* @source 0x80143FC8 @kind bss */
/* Shared CLUT-bank byte read as the trailing argument of the GAME00
 * shaded-panel helper drawFramedTexturedPanel by the AREA030 cell drawers. */
extern u8  D_80144952;    /* @source 0x80144952 @kind unknown */
/* Local AREA030 scratch-mode handler table indexed by scratch cursor byte 1. */
extern void (*D_801E1D80[])(void);
extern void (*D_801E1D10[])(void);
extern void (*D_801E1D20[])(void);
extern void (*D_801E1D78[])(void);
extern void (*D_801E1D8C[])(void);
extern void (*D_801E1F78[])(void);
extern void (*D_801E1F44[])(void);
extern u16 D_801E1DA0[]; /* @source 0x801E1DA0 @kind data */
typedef struct Area030LinePair {
  u32 tag;
  u8 r0;
  u8 g0;
  u8 b0;
  u8 pad_07;
  s16 x0;
  s16 y0;
  s16 x1;
  s16 y1;
  s16 x2;
  s16 y2;
  s16 x3;
  s16 y3;
} Area030LinePair;
typedef struct Area030StepEntry {
  s32 first;
  s32 second;
} Area030StepEntry;
extern Area030StepEntry D_801E1D58[]; /* @source 0x801E1D58 @kind table */
extern u8 D_80143FEC[];   /* @source 0x80143FEC @kind unknown */
extern u8 D_80144124[];   /* @source 0x80144124 @kind unknown */
extern u8 D_80145E93;    /* @source 0x80145E93 @kind unknown */
extern u32 D_80145EC4;    /* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC8;    /* @source 0x80145EC8 @kind unknown */
extern u32 D_801440BC;    /* @source 0x801440BC @kind unknown */
/* Shared word cleared by the AREA030 pad decision step 0x801DA638 before it
 * queues sound cue 0x200. No reader of this word is lifted in the target, so
 * the role stays address-anchored; only the stored width is proven. */
extern u32 D_801440C8;    /* @source 0x801440C8 @kind unknown */
/* Per-mode pair of words copied into the AREA030 scratch record at +0x14 and +0x20. */
typedef struct Area030ModePair { u32 first; u32 second; } Area030ModePair;
extern Area030ModePair D_801E1D38[]; /* @source 0x801E1D38 @kind data */
s32 stepActiveWorkSequence(void);
void advancePanelScroll(void);
void drawEncodedTextAt(s32 arg0, s32 arg1, s32 arg2, u32 arg3, u32 arg4);
/* SLUS 8x8 glyph-string drawer at 0x80150098: signed screen coordinates,
 * the low six CLUT bits and a read-only string. Handles newline and space
 * bytes; callers perform formatting. */
void func_80150098(s16 x, s16 y, u16 clut, const u8* input);
/* Per-index byte and word pair consumed by the encoded-text draw at 0x8014F800.
 * byte_00 is the message-run token limit, word_04 the run cursor the AREA030 panel
 * message step 0x801D3A54 repoints to, byte_08 the attribute compared against the
 * 0xFF sentinel and byte_09 the reveal-timer reload. */
typedef struct Area030PanelEntry { u8 byte_00; u8 pad_01[3]; u32 word_04; u8 byte_08; u8 byte_09; u8 pad_0A[2]; } Area030PanelEntry;
extern Area030PanelEntry D_801E1E78[]; /* @source 0x801E1E78 @kind data */
extern u8 D_801E1F20[][4]; /* @source 0x801E1F20 @kind data */
extern u8 D_801E1F14[]; /* @source 0x801E1F14 @kind data */
u32 findFreeRecord(void);
/* Shared palette-stage serial byte published as 1 by the AREA030 color-row
 * builder buildColorRowAndStartTintFadeWorld00Area03004_801D130C after it
 * rewrites the 0x800396C0 color row. */
extern u8  D_80145988;    /* @source 0x80145988 @kind unknown */
/* Signed limit byte (and its +3 companion byte) of the AREA030 message state
 * advanced by updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004. */
extern s8  D_80143FCF;    /* @source 0x80143FCF @kind unknown */
/* Front-end step word and its +4 companion word: latchPadMaskAndControlSharedStepWorld00Area03004 steps
 * D_80143FD8 back by 0x4000 while D_80143FDC stays clear, and reads the
 * companion as a plain scalar at its own address. */
extern s32 D_80143FD8;    /* @source 0x80143FD8 @kind unknown */
extern s32 D_80143FDC;    /* @source 0x80143FDC @kind unknown */
/*
 * Motion view of the leading 0x74-byte front-end entry record at 0x80143FC8,
 * the row the AREA030 overlay drives as its own panel state: byte +0x0B is the
 * latch that gates the pad group, words +0x0C/+0x10/+0x14 the step triple the
 * pad arms rewrite, words +0x34/+0x38 supply the coordinates to sampleMapHeight; signed halfword +0x3E stores the sampled map height as a counter. The three cells the pad
 * group clears have separate scalar views here at their recorded addresses.
 */
extern s32 D_80143FD4;    /* @source 0x80143FD4 @kind unknown */
/* Per-slot stored screen position halfwords: func_801D6128 writes the
 * 0xF0/0xA6 pair through the same index base as the D_80143FCD step bytes, and
 * the AREA030 name step offsets them by (+0x0A, -0x18) for its icon strip. */
extern u16 D_80143FF6;    /* @source 0x80143FF6 @kind unknown */
extern u16 D_80143FF8;    /* @source 0x80143FF8 @kind unknown */
extern s32 D_80143FFC;    /* @source 0x80143FFC @kind unknown */
extern s32 D_80144000;    /* @source 0x80144000 @kind unknown */
extern s16 D_80144006;    /* @source 0x80144006 @kind unknown */
extern u8  D_8014403D;    /* @source 0x8014403D @kind unknown */
/* AREA030 step counter byte beside D_8014403D. Original body 0x801DA92C
 * addresses it through a materialized base register. This declaration provides
 * an array view of that address. */
extern u8  D_8014403E[];  /* @source 0x8014403E @kind unknown */
extern u8  D_801440B1;    /* @source 0x801440B1 @kind unknown */
extern u8  D_80144FE4[];  /* @source 0x80144FE4 @kind unknown */
extern u8  D_80144281;    /* @source 0x80144281 @kind unknown */
extern u8  modeByte;      /* @source 0x80144125 @kind bss */
extern u8  D_8014412A;    /* @source 0x8014412A @kind unknown */
extern u8  D_8014412B;    /* @source 0x8014412B @kind unknown */
extern u8  D_80144199[];  /* @source 0x80144199 @kind unknown */
/* @source 0x80144199 @kind unknown: byte-scalar view of the shared AREA030
 * selection gate, alongside the array view at the same address. */
extern u8  D_80144199_BYTE;
extern u8  D_8014419E;    /* @source 0x8014419E @kind unknown */
extern u8  D_80144286;    /* @source 0x80144286 @kind unknown */
/* Shared AREA030 panel flag byte: the 0x801DCCF4 handler masks it to seven
 * bits and publishes the masked value back, and selects its main-executable
 * VRAM halfword through its bit 0. */
extern u8  D_80144287;    /* @source 0x80144287 @kind unknown */
/* Byte cleared by AREA030's pressed-mask arm at 0x801DC93C when it
 * publishes 4 into work-record dispatch byte 3. Its encoded meaning and
 * storage kind remain unverified; no other current target body reads it. */
extern u8  padPressedMaskTransitionClearedByteState; /* @source 0x8014428B @kind unknown */
/* Shared AREA030 pad gate/latch byte directly after D_80144287: 0x801DD7CC
 * republishes 0xFF while the pad Cross bit D_80145AA8/0x4000 is set and 1
 * while the Triangle bit 0x1000 is set, and both 0x801DD7CC and 0x801D56F4
 * bail out while it reads nonzero. */
extern u8  gateByteWorld00Area03004_80144288; /* @source 0x80144288 @kind unknown */
/* Shared AREA030 list index stepped by the pad handlers: 0x801DD7CC cycles it
 * over 0..5 on the Cross/Triangle bits and 0x801DD664 cycles it over 0..0x16
 * before publishing 0x80144281 + 1. */
extern u16 D_801442BC;    /* @source 0x801442BC @kind unknown */
/* Shared AREA030 halfword read as the step source by 0x801DD664: that handler
 * publishes D_801442BE plus or minus one into D_801442BC, so the read and the
 * write of one step sit in adjacent cells. No other lifted body reads it. */
extern u16 D_801442BE;    /* @source 0x801442BE @kind unknown */
/* Halfword published by AREA030 record-selection steps as 0xFFFF or a
 * selected record byte plus four. The step at 0x801DCDB8 writes 0xFFFF on
 * cancel or a zero chosen byte; retained 0x801DCFD0 tests record bytes.
 * Downstream interpretation, other writers and storage stability remain
 * unresolved. This target-local name describes publication without assigning
 * a global owner, object extent or complete selection domain. */
extern u16 selectedRecordValueState;    /* @source 0x801442AC @kind unknown */
extern u8  D_80145E92;    /* @source 0x80145E92 @kind unknown */
extern u8  D_80145E93;    /* @source 0x80145E93 @kind unknown */
/* Shared AREA030 color-block row index byte:
 * buildColorRowAndStartTintFadeWorld00Area03004_801D130C reads it once per
 * copied element and uses it to select the 0x40-byte row of the 0x80037800
 * color block that feeds the 0x800396C0 color row. */
extern u8  D_80145EB7;    /* @source 0x80145EB7 @kind unknown */
/* Shared AREA030 panel-template bytes published into the scratch work record
 * by publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0 (offsets 0x2A
 * and 0x49..0x4B) and its shared word/halfword
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
 * initializer resetWorkStepsAndSelectStateWorld00Area03004 gates its resource request on byte 0x79 of the
 * record it points at. */
extern u8* D_80146250;    /* @source 0x80146250 @kind unknown */
extern void (*D_801E2000[])(void); /* @source 0x801E2000 @kind table */
extern void (*D_801E2014[])(void); /* @source 0x801E2014 @kind table */
extern void (*D_801E2048[])(void); /* @source 0x801E2048 @kind table */
/* Four-by-eight cell table at 0x801E2050 of the AREA030 grid handler
 * 0x801D6684: one byte per cell, 0xFF ending the drawn run of a row, and the
 * value of a live cell (0 or 1) selecting both its shade and the one-character
 * run the cell is drawn with. The handler walks it eight bytes at a time, once
 * per grid row. */
extern u8 D_801E2050[][8]; /* @source 0x801E2050 @kind rodata */
/* One-character text runs at 0x801E2070 indexed by a 0x801E2050 cell value:
 * 0x92 for the value 0 and 0x81 for the value 1, each followed by its
 * terminator. */
extern u8 D_801E2070[][2]; /* @source 0x801E2070 @kind rodata */
extern void (*D_801E2074[])(void); /* @source 0x801E2074 @kind table */
/* Four-byte AREA030 step records at 0x801E207C walked by stepPanelMotionScriptAndDrawMarkerRowWorld00Area03004 through
 * the work-record step byte (offset 10 of the record published at the scratchpad
 * cursor 0x1F800044): byte 0 is the 0xFF end marker and the 0x80/0x7F request
 * bits, byte 1 the step countdown copied into work byte 9, byte 2 the delta
 * integrated into work word 0x0C, and byte 3 the delta integrated into work word
 * 0x10 whose sign selects the clampMarkerPositionsAndDrawPanelRowWorld00Area03004 marker mode. The byte-0 and byte-2
 * request and subtraction sites read these bytes unsigned, so they reach the
 * table through an address-derived byte view of this same symbol. */
typedef struct Area030StepRecord {
  s8 unk_00;   /* +0x00: 0xFF end marker; bit 7 gates the func_8014D978 call */
  u8 count_01; /* +0x01: step countdown published into work byte 9 */
  s8 unk_02;   /* +0x02: delta added to work word 0x0C and work byte 6 */
  s8 unk_03;   /* +0x03: delta added to work word 0x10 */
} Area030StepRecord;
ASSERT_SIZE(Area030StepRecord, 4);
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
ASSERT_SIZE(Area030StepRecordBytes, 4);
extern Area030StepRecord D_801E207C[]; /* @source 0x801E207C @kind table */
extern void (*D_801E20C4[])(void); /* @source 0x801E20C4 @kind table */
/* Second four-byte step chain, walked by 0x801D6F34: 17 records occupy
 * [0x801E20CC, 0x801E2110), followed by FF 00 00 00 at 0x801E2110.
 * Byte 0 carries the func_8014D978 gate and request id; byte 1 reloads work
 * byte 9. Bytes 2 and 3 supply deltas for work words 0x0C and 0x10. The latter's
 * sign selects the marker mode and counter step. Request reads use unsigned
 * bytes; end-marker, delta and marker-sense reads cast individual bytes to s8. */
extern Area030StepRecordBytes panelStepRecordTableWorld00Area03004_801E20CC[]; /* @source 0x801E20CC @kind table */
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
/* Per-index halfword limits compared by advanceWorkModeAtIndexedOffsetElseStepSequenceWorld00Area03004 with the work-record
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
 * by the record stage byte at scratch offset 3 by
 * dispatchStateByte3Table1d00World00Area03004_801D12C8. The three
 * code pointers observed at 0x801E1D00/0x801E1D04/0x801E1D08 are the
 * consecutive stage handlers 0x801D130C, 0x801D1434 and 0x801D1538. It sits
 * inside the T_801E1CD4 data blob, preceded there by an unrelated pointer
 * block, so the table base is the address the original code indexes from. */
extern void (*D_801E1D00[])(void); /* @source 0x801E1D00 @kind table */
/* Four-byte AREA030 texture-cell u0 table (bytes C0 D0 E0 D0) indexed by bits
 * 3-4 of the shared frame counter D_80143E6C by drawCounterIndexedSprite16x16World00Area03004. */
extern u8 D_801E1D0C[]; /* @source 0x801E1D0C @kind table */
extern u8  D_801E28C4[];  /* @source 0x801E28C4 @kind rodata */
extern u8  D_801E28D0[];  /* @source 0x801E28D0 @kind rodata */
/* Four 8-byte AREA030 text slot records read by updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004: a text pointer
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
extern u8  D_80144002;    /* @source 0x80144002 @kind unknown */
extern u8  D_80145AD4[];  /* @source 0x80145AD4 @kind unknown */
/* Shared main-executable per-slot pair the AREA030 lure column walks over the
 * 0x80 slots of the front-end inventory: D_801451CC carries the equipment
 * record index of each slot and D_801453CC the slot count it formats. */
extern u8  D_801451CC[];  /* @source 0x801451CC @kind table */
extern u8  D_801453CC[];  /* @source 0x801453CC @kind table */
/* Shared main-executable mode byte the AREA030 lure column reads at index zero:
 * it marks the selected row only while the byte reads two. */
extern u8  D_80145E94[];  /* @source 0x80145E94 @kind table */
extern const char D_801D0C04[]; /* @source 0x801D0C04 @kind rodata */
/* NUL-terminated "%d/6" format for the selected step counter. */
extern const char D_801D0C08[]; /* @source 0x801D0C08 @kind rodata */
/* Two-character rodata label "23" (bytes 0x32 0x33 0x00 0x00) that the AREA030
 * entry-panel counter row drawEntryPanelCounterRowWorld00Area03004 submits as the text-pointer argument of
 * the shared text drawer func_8014FF0C. */
extern const char D_801D0C10[]; /* @source 0x801D0C10 @kind rodata */
/* NUL-terminated "*%2d" format for lure slot counts of two or more. */
extern const char D_801D0C14[]; /* @source 0x801D0C14 @kind rodata */
/* Twelve-byte rodata run "GearDataRule" that the AREA030 status-cell row
 * drawLabeledStatusCellsWorld00Area03004_801D7C10 submits as three separate
 * 4-character label runs through the shared text drawer drawEncodedTextAt. */
extern const char D_801E21C0[]; /* @source 0x801E21C0 @kind rodata */
/* Four-character rodata label "LURE" (bytes 0x4C 0x55 0x52 0x45), the second
 * word of the LUREEQUIPGUIDE run, that the AREA030 lure column
 * drawLureListColumnWorld00Area03004_801D8364 submits as the column title of
 * the shared text drawer drawEncodedTextAt. */
extern const char D_801E21D0[]; /* @source 0x801E21D0 @kind rodata */
/* Five-character rodata label "EQUIP" (bytes 0x45 0x51 0x55 0x49 0x50), the
 * third word of that LUREEQUIPGUIDE run, that drawEquipmentGuideColumn submits
 * as the header of the two equipment rows. */
extern const char D_801E21D4[]; /* @source 0x801E21D4 @kind rodata */
/* Five-character rodata label "GUIDE" (bytes 0x47 0x55 0x49 0x44 0x45), the
 * last word of that LUREEQUIPGUIDE run, that drawEquipmentGuideColumn submits
 * above the main-RAM guide icon row. */
extern const char D_801E21D9[]; /* @source 0x801E21D9 @kind rodata */
extern u16 D_801E218C[]; /* @source 0x801E218C @kind table */
extern u16 D_801E218E[]; /* @source 0x801E218E @kind table */
extern u16 D_801E2190[]; /* @source 0x801E2190 @kind table */
extern u8 D_801E2192[]; /* @source 0x801E2192 @kind table */
extern u16 D_801E2194[]; /* @source 0x801E2194 @kind table */
/* Six-entry halfword AREA030 step-icon table: the selected step word of the work
 * record indexes it and the entry names the main-executable VRAM sprite drawn
 * for that step. */
extern u16 D_801E2114[]; /* @source 0x801E2114 @kind table */
/* 54-entry halfword AREA030 step-strip sprite table: one sprite id per strip
 * frame with 0xFFFF marking a gap, walked circularly by
 * drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4 from the work
 * record's strip clock word as the strip animates. */
extern u16 D_801E2120[]; /* @source 0x801E2120 @kind table */
extern u8  D_801E2384[];  /* @source 0x801E2384 @kind unknown */
extern u8  D_801E2388[];  /* @source 0x801E2388 @kind unknown */
extern u8  D_801E238C[];  /* @source 0x801E238C @kind unknown */
extern u8  D_801E2390[];  /* @source 0x801E2390 @kind unknown */
extern u8* g_PrimCursor;    /* @source 0x8014598C @kind bss */
extern s32 D_8014421C;     /* @source 0x8014421C @kind unknown */
/* AREA030 record-selection progress byte updated by 0x801DB140. A fresh
 * 0xC000 press or nonzero countdown increments it; either nested 0x40 arm
 * can increment it again. The 0x3000 arm decrements it; standalone 0x40
 * increments it. Other paths can leave it unchanged. */
/* @source 0x80144216 @kind unknown: existing unsized byte-array view.
 * Element zero and the scalar below name the same byte. Its initial value,
 * complete domain, storage owner and downstream interpretation are unresolved. */
extern u8  recordSelectionProgressState[];
extern u8  recordSelectionProgressByteState; /* @source 0x80144216 @kind unknown */
/* Shared AREA030 selection-step byte at the head of the 0x8014421C region: the
 * dispatch step requestStateTwoOnGateThreeThenStepSequenceWorld00Area03004 advances it once per pass through its gate. */
extern u8  D_8014420D;     /* @source 0x8014420D @kind unknown */
extern u16 D_80145AA8;     /* @source 0x80145AA8 @kind unknown */
extern u16 D_80145AC2;     /* @source 0x80145AC2 @kind unknown */
extern u16 D_80145AC4;     /* @source 0x80145AC4 @kind unknown */
extern u16 D_80145AA4;     /* @source 0x80145AA4 @kind unknown */
extern u8  D_8014504C[];   /* @source 0x8014504C @kind unknown */
extern u8  D_8014524C[];   /* @source 0x8014524C @kind unknown */
extern u8  D_801E31F0;     /* @source 0x801E31F0 @kind unknown */
extern u8  D_801E31F4;     /* @source 0x801E31F4 @kind unknown */
extern u8  D_801E31F8;     /* @source 0x801E31F8 @kind unknown */
/* Saved previous selection byte, read by playCueForChangedSelectedValueWorld00Area03004 as a plain scalar.
 * The selection callers save 0x801E31F8 here before changing the selection. */
extern u8  D_801E31FC;     /* @source 0x801E31FC @kind unknown */
/* AREA030 text slot tables walked by updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004: D_801E31E0 holds the four
 * per-slot character counters and D_801E31E4 the four per-slot frame timers. */
extern u8  D_801E31E0[];   /* @source 0x801E31E0 @kind unknown */
extern u8  D_801E31E4[];   /* @source 0x801E31E4 @kind unknown */
/* Nonzero byte flag at the end of this overlay's data; read by
 * drawExtraIconWhenEnabledAndFrameBitThreeSetWorld00Area03004 to gate its extra icon and cleared by func_801DBBAC while
 * the shared pad mask 0x80145AA8/0x50 is set. */
extern u8  D_801E3214;     /* @source 0x801E3214 @kind unknown */
extern u8 D_80145026;      /* @source 0x80145026 @kind unknown */
/* Companion byte cleared together with entry-ID byte 0x80145026 by
 * consumeEntryUnitOrClearPairedBytesThenConfirmSelectionStepWorld00Area03004 when the matching-entry amount consumer reports failure.
 * Read as a plain scalar at its own address; broader purpose is unknown. */
extern u8 D_80145027;      /* @source 0x80145027 @kind unknown */
extern u8 D_80145028;      /* @source 0x80145028 @kind unknown */
extern u8 D_801E2720[];    /* @source 0x801E2720 @kind unknown */
extern u8 D_801E2748[];    /* @source 0x801E2748 @kind unknown */
/* Latch byte of the AREA030 work-record entry group: cleared by the entry
 * handler at 0x801D9B9C, armed with 1 by the record walker func_801DE82C
 * (sb at 0x801DE8E4) and cleared again by func_801DEBC4 (sb at 0x801DEBEC).
 * Role not established; kept address-anchored. */
extern u8  D_801E3200;     /* @source 0x801E3200 @kind unknown */
/* Latch used by selectWorkModeOrStepOnPadWithMatchingSlotLatchWorld00Area03004 after the hasAtLeastTwentyTwoMatchingSlotsWorld00Area03004 matching-slot test.
 * A passing test sets it only when zero; a failed test clears it. */
extern u8  D_801E3204;      /* @source 0x801E3204 @kind unknown */
extern u8  D_801E3208;      /* @source 0x801E3208 @kind unknown */
extern s8* D_801E320C;     /* @source 0x801E320C @kind unknown */
extern u8* D_801E3210;     /* @source 0x801E3210 @kind unknown */

/* Global route/step table pair owned by the main executable: route entry `i`
 * is the step pair D_80181AC0[i * 2] / D_80181AC4[i * 2]. deriveAndReflectWorkRouteStepsThenReindexWorld00Area03004
 * reads one entry through the work-record route byte and scans the eight
 * entries again to re-index it. */
extern s32 D_80181AC0[];  /* @source 0x80181AC0 @kind table */
extern s32 D_80181AC4[];  /* @source 0x80181AC4 @kind table */

typedef struct Area030SlotState {
  s32 value;
  u8 pad_04[0x94];
} Area030SlotState;

extern Area030SlotState D_801468A4[]; /* @source 0x801468A4 @kind unknown */

/* Shared 0x98-byte work records at 0x80146888 (0x1E slots). The AREA030
 * handlers walk them, publish the active record as the scratchpad cursor at
 * 0x1F800044 and as D_80146884, then dispatch the record's handler byte
 * through the overlay-local handler table. */
typedef struct Area030WorkRecord {
  u8  flags_00;      /* +0x00: record flags; 0x20 gates the level update */
  u8  handler_01;    /* +0x01: handler index into D_801E2314 */
  u8  unk_02[0x1A];  /* +0x02-0x1B */
  s32 unk_1C;        /* +0x1C: signed cell the 0x801DB140 cross-pad arm steps */
  u8  unk_20[0x14];  /* +0x20-0x33 */
  s32 x_34;          /* +0x34: signed record x the area handlers compare */
  s32 y_38;          /* +0x38: signed record y the area handlers compare */
  u8  unk_3C[2];     /* +0x3C-0x3D */
  s16 counter_3E;    /* +0x3E: signed countdown the level derives from */
  u8  unk_40[0x1D];  /* +0x40-0x5C */
  s8  level_5D;      /* +0x5D: level derived from counter_3E */
  u8  unk_5E;        /* +0x5E: level mirror */
  u8  unk_5F;        /* +0x5F: level mirror */
  u8  unk_60[0x2C];  /* +0x60-0x8B */
  s16 limit_8C;
  s16 value_8E;
  u8  unk_90[8];     /* +0x90-0x97 */
} Area030WorkRecord;
ASSERT_SIZE(Area030WorkRecord, 0x98);

extern Area030WorkRecord* D_80146884; /* @source 0x80146884 @kind unknown */

extern Area030WorkRecord D_80146888[]; /* @source 0x80146888 @kind unknown */
extern void (*D_801E2314[])(void); /* @source 0x801E2314 @kind table */
/* Eight two-byte AREA030 step-mode rows at 0x801E2330, selected by the route
 * index byte 0x08 of the scratch work record: the leading byte is the id the
 * step at 0x801DE82C hands to the shared selector func_8014D8D4 and the
 * step at 0x801DEBC4 selects an active-work sequence through selectActiveWorkSequence, the trailing byte
 * the value the step at 0x801DE82C mirrors into work byte 0x2A. */
typedef struct Area030StepModeRow {
  u8 request_00;
  u8 byte_01;
} Area030StepModeRow;
ASSERT_SIZE(Area030StepModeRow, 2);
extern Area030StepModeRow D_801E2330[]; /* @source 0x801E2330 @kind table */

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

/* AREA030 per-slot scale records: the motion/step handler at 0x801DF334 forms
 * `0x801E2AB8 + index * 36` and the sibling slot handlers read halfwords at
 * +0x10/+0x12 and bytes at +0x1A..+0x20 of the same records. Only the fields
 * with a proven consumer are named here. */
typedef struct Area030ScaleRecord {
  u8 unk_00[0x10];
  /* +0x10: entry loaded through applyMainLookupRecord by the entry step at
   * 0x801DE5EC, and by the slot handlers at 0x801DEBC4 and 0x801DF334, for the
   * record selected by work byte 0x06. */
  u16 entry_10;
  /* +0x12: entry loaded through applyMainLookupRecord by the work-record entry-phase
   * start at 0x801E0124 for the record selected by work byte 0x06. */
  u16 entry_12;
  u8 unk_14[6];
  u8 unk_1A;     /* +0x1A: heading floor and counter scale read by the entry
                  * step at 0x801DE5EC */
  u8 unk_1B;     /* +0x1B: single-byte scale row read by deriveAndReflectWorkRouteStepsThenReindexWorld00Area03004 */
  u8 unk_1C[3];
  u8 limit;      /* +0x1F: highest slot amount that still scales */
  u8 unk_20[2];
  u16 value;     /* +0x22: value reported at the full limit */
} Area030ScaleRecord;
ASSERT_SIZE(Area030ScaleRecord, 0x24);

extern Area030ScaleRecord D_801E2AB8[]; /* @source 0x801E2AB8 @kind table */

/* Scratchpad work-record cursor cell; reloaded per store group by the
 * AREA030 handlers (volatile cell, plain RAM pointee). */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind bss */

/* Scoped companion-call ABI for func_801E0C20; not a callback contract. */
void dispatchArea030CompanionHandler(void); /* @source 0x800F500C */
void projectAndQueueActiveWork(void);
u8   func_8014D978(void);
/* Executable matching-entry amount consumer at 0x801665A0. AREA030 sites
 * pass (group 3, entry ID, 1, 0) and discard the result or test its low byte.
 * Preserve this scoped four-argument byte-result view; live group ownership
 * and initialization are not established. */
u8   func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
/* Target-local pad-mask gate at 0x801DDF7C: its `and` of D_80145AA8 with the
 * caller's argument selects the gate path and it returns nonzero on the taken
 * path (a local-mode-2 request plus scratch bytes 2/3). Only the argument matters to
 * the AREA030 call site, which discards the gate result. */
u32  enterWorkModeEightIfInputMaskSetWorld00Area03004(u16 mask);
void queueUnprojectedActiveWork(void);
/* Target-local AREA030 selection helpers at 0x801DB04C and 0x801DB0AC: the
 * pad-mode dispatch step 0x801DB140 calls each with no arguments and discards
 * the result. */
extern void selectRecordValueAndSlot(void); /* @source 0x801DB04C */
extern void setActiveSlotValueFromFrameMask(void); /* @source 0x801DB0AC */
u32  func_8014D8D4(u8 arg0);
void applyMainLookupRecord(s32 arg0);
/* Shared active-work sequence selector at 0x8014D6B8: the AREA030 record initializer
 * initializeWorkWordPairAndCountdownThenAdvanceHandlerWorld00Area03004 calls it with the sequence selector 3 (direct jal at 0x801D64B8). */
void selectActiveWorkSequence(s32 arg0);
/* Shared resource-request helper at 0x8014D6DC: the AREA030 step initializer
 * resetWorkStepsAndSelectStateWorld00Area03004 calls it with (0xA, 2) when the flag byte at D_80146250 + 0x79
 * is clear. The shipped executable body reads the scratchpad cursor at
 * 0x1F800044 and masks its first argument with 0x80, confirming the
 * two-argument form (direct jal at 0x801D9DC8). */
void func_8014D6DC(s32 arg0, s32 arg1);
/* Shared overlay reset at 0x80153DE8: the AREA030 dispatch step 0x801DC74C runs
 * it with no arguments between the channel-pair commit and the entry-table
 * clear, and the scenario state sequences reset the overlay through it too. */
void func_80153DE8(void);
void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);
int  func_8017E3F4(char* buffer, char* fmt, ...);
void drawStatusMarkerFrameWorld00Area03004(s16 originX, s16 originY);
void drawFixedCellSprite8x8World00Area03004(s16 arg0, s16 arg1, u8 arg2);
void initFlatQuadPrimitive(void* arg0);
/* Target-local icon-row/numeric-label drawer at 0x801E1320: the AREA030 panel
 * step calls it with the constant top-left corner (176, 156). */
void drawSlotValueSummaryAndThresholdIconsWorld00Area03004(s16 x, s16 y);
/* Draw sprite 0x0C and an optional indexed byte stream. The panel step calls
 * it at x=8, y=148 with run index 66. */
void drawSpriteAndOptionalIndexedRunWorld00Area03004(s32 x, s32 y, u16 runIndex);
void submitTpageDrawMode(s32 arg0, s32 arg1);
u32 sumValuesForNonzeroSlotAmountsWorld00Area03004(void);
extern u8 D_801454EC; /* @source 0x801454EC @kind unknown */
u8*  appendIndexedSpriteWorld00Area03004(s32 spriteIndex, s32 drawMode, s16 x, s16 y);
void drawIndexedPanelQuadWithHeightWorld00Area03004(s16 x, s16 y, u8 height, u8 recordIndex);
void drawWidePanelFrameWorld00Area03004(s16 x, s16 y);
/* Target-local stretched panel column at 0x801D8248: emits one 8-pixel-wide
 * grey quad of the requested height from the 8 by 4 texel slice at (0x58,
 * 0x9A) of the panel texture page. The AREA030 status panel drawer at
 * 0x801D7EB0 runs it at (x + 0x80, y + 0x1C) with the panel height 0x72, and
 * the lure column drawLureListColumnWorld00Area03004_801D8364 runs it at
 * x + 0x80 with a height it clamps to at most 0x72. */
void drawStretchedPanelColumnWorld00Area03004_801D8248(s16 x, s16 y, u8 height);
void drawSemiTransPaletteRectangleFill(u16 x, u16 y, u16 w, u16 h);
void drawSemiTransPaletteRectangleBorder(u16 x, u16 y, u16 w, u16 h, u8 mode);
void drawFilledPanelWithInsetBorderWorld00Area03004(s16 arg0, u16 arg1, s16 arg2, s16 arg3, u8 arg4);
void drawLabeledStatusCellsWorld00Area03004_801D7C10(u16 x, u16 y, u8 mask);
/* Target-local AREA030 status-panel cell drawer at 0x801D7D68: the status-cell
 * row above draws its three cells through it with the per-cell flag. */
void drawGroupedCellSpritesWorld00Area03004(s16 x, s16 y, u8 groupIndex);
/* Target-local AREA030 equipment column at 0x801D88E0: draws the two status
 * boxes, the EQUIP and GUIDE rodata headers, the two equipment records of the
 * shared index bytes D_80145028/D_80145026 and the main-RAM guide icon of the
 * work-record halfword +0x2C. The sliding status panel at 0x801D4680 and the
 * fixed side panels call it at (16, 64), or at (16 - 45 * count, 64) once the
 * left box covers several entries. */
void drawEquipmentGuideColumn(s16 x, s16 y);
/* Target-local drawer of the two stacked AREA030 status boxes at 0x801D8D70:
 * the left status column of drawEquipmentGuideColumn runs it at its own origin
 * and the fixed (16, 64) panel steps call it through that column drawer. */
void drawStatusBoxPair(s16 x, s16 y);
void drawListColumnFrameWorld00Area03004(s16 x, s16 y);
void drawRodListColumnWorld00Area03004_801D7EB0(s16 arg0, s32 arg1);
/* Target-local drawer of the AREA030 lure column at 0x801D8364: the fixed
 * (164, 64) panel step and the sliding panel steps call it beside the
 * drawEquipmentGuideColumn equipment column at the same origin row. */
void drawLureListColumnWorld00Area03004_801D8364(s16 x, s16 y);
/* Target-local drawer of one panel message token at 0x801D9740: the AREA030
 * message steps submit the current token of the run cursor with the reveal
 * width and the pen column. */
void drawMessageTokenCellQuad(u8 arg0, u8* arg1, u8 arg2, s16 arg3);
/* Target-local compact AREA030 panel drawer at 0x801E1758: the AREA030 panel
 * steps call it with the computed horizontal origin and the constant 0x56. */
void drawArea030CompactPanel(s16 x, s16 y);
void func_80196070(void);
/* Same-binary entry-table clearer at 0x801960C0 (clearRecord in the game00
 * view): clears the five head bytes of one 0x74-byte record slot. The AREA030
 * reset passes a slot index, which the u8 parameter truncates. */
void clearRecord(u8 record_index);
/* Same-binary frontend selection-effect starter at 0x8015D4F8 (executable
 * space, called by the AREA030 selection step with (0, 0, 100, 8)). */
void func_8015D4F8(u8 arg0, u8 arg1, s32 arg2, s32 arg3);
/* Resident audio gain request at 0x8015D49C. When byte 0x80144954 is zero,
 * it maps arg0 through a signed-byte table and prepares an indexed gain
 * increase. The AREA030 work-record step calls it with (0, 0, 0x14, 0x10).
 * The gate's meaning and the preceding halfword writes remain unresolved. */
extern void requestMappedAudioGainIncrease(u32 arg0, u32 arg1, s32 arg2, s32 arg3); /* @source 0x8015D49C */
/* Same-binary frontend selection-effect helper at 0x8015D404 (executable
 * space): the AREA030 dispatch step requestStateTwoOnGateThreeThenStepSequenceWorld00Area03004 calls it with both arguments
 * zero, which is the same shape the commu00 and world00/area007 views declare
 * as (effect index, value). */
void func_8015D404(u32 arg0, s32 arg1);
/* Same-binary sound-cue dispatcher at 0x8015DF18 (executable space): the
 * AREA030 panel step calls it with the fixed cue 0x102. */
void func_8015DF18(u16 arg0);
/* Main-exe pad direction decoder (executable space): the AREA030 work-byte-4
 * step at 0x801DCDB8 submits the shared pad state D_80145AA8 masked by 0x5000
 * and tests the returned bits 0x5000, the same call shape the shop00 and
 * sisyou00 call sites declare as s32 func_8014E640(u32). Body not lifted in
 * this target. */
s32 func_8014E640(u32 arg0);
/* GAME00 shaded-panel rectangle helper at 0x801AE3F0:
 * the AREA030 cell drawer submits its 0x28 by 0x10 panel rectangle at the
 * two-pixel inset corner of the requested origin, with the shared CLUT-bank
 * byte D_80144952 as the trailing argument. Body not lifted in this target. */
void drawFramedTexturedPanel(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                   u32 arg5);
/* Target-local prototype for the co-resident front-end entry probe at
 * 0x801BDD58 (direct `jal` at 0x801E0738 in checkWorkEntryGateOrSelectHandlerOneWorld00Area03004): it takes the
 * record x at +0x34, y at +0x38 and signed counter at +0x3E, a byte range and
 * the entry-table pointer 0x80143FC8 in its fifth stack argument, and reports
 * a byte-valued match. Body not lifted in this target. */
/* Consumes reference as a signed 16-bit value. */
u8 matchesProjectedWorkBounds(s32 x, s32 y, s32 reference, s32 range, void* work);
s32 sampleMapHeight(s32 arg0, s32 arg1);
s32  calculateScaledSlotValueForAmountWorld00Area03004(s32 slotIndex, s32 slotAmount);
/* Target-local AREA030 panel combination drawer at 0x801E19CC: the entry-panel
 * slide steps call it with the sliding 0x44 +/- 0x50 * count origin on the
 * fixed 0x4E row. */
void drawArea030Panel(s16 x, s16 y);
s32  hasAtLeastTwentyTwoMatchingSlotsWorld00Area03004(void);
void updateWorkScrollWordsWorld00Area03004_801E084C(void); /* per-record scroll updater on the 0x1F800044 cursor */
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
void dispatchActiveWorkRecordsAndUpdateLevelsWorld00Area03004(void);
/* Target-local per-record scroll updater loop at 0x801E09B4. */
void updateActiveWorkRecordScrollWordsWorld00Area03004(void);
/* AREA030 overlay per-frame entry at 0x801E0BD0: invoked once by the
 * main-executable front-end substate-4 handler func_801984E8 (jal 0x80198578).
 */
void updateArea030OverlayFrame(void);
/* Shared frame-service chain at 0x801E0C40: invoked by the front-end substate-4
 * wait loops func_801985C8 (jals 0x80198614, 0x80198694). */
void runArea030FrameServices(void);

void publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0(void);
void dispatchStateByte3Table1d00World00Area03004_801D12C8(void);
void dispatchStateByte2Table21e0World00Area03004(void);
void dispatchStateByte3Table2210World00Area03004(void);
void advanceCounterAndWorkModeIfStateIsFourWorld00Area03004(void);
void dispatchStateByte3Table221cWorld00Area03004(void);
void dispatchStateByte3Table2000World00Area03004(void);
void dispatchStateByte3Table2014World00Area03004(void);
void dispatchStateByte3Table2048World00Area03004(void);
void drawStatusHeaderCounterAndMarkerFrameWorld00Area03004(s16 x, s16 y);
void drawBlueGradientQuad16x32World00Area03004(s16 x, s16 y, u8 drawMode);
void drawCounterIndexedSprite16x16World00Area03004(s16 x, s16 y, u8 drawMode);
/* @source 0x8017AA08 — shared 16x16 sprite helper that sets packet length 3 and command 0x7C verified in the shipped executable; absent from the SDK map, so it
 * stays target-local. */
void initSprite16Primitive(SPRT_16* arg0);
void drawGreenGradientBar(s16 arg0, s16 arg1, s16 arg2, u8 arg3);
void queueIconStrip(s16 arg0, s16 arg1, u8 arg2, u8 arg3);
void copySharedWorkParametersOrClearWorkFlagsWorld00Area03004(void);
void drawMirroredPanelCellQuadWorld00Area03004_801D2C34(s16 arg0, s16 arg1,
                                                        s8 arg2, u8 arg3);
void drawPanelCellAndOptionalBlinkingHighlightWorld00Area03004(s16 x, s16 y, u8 width, s8 uOffset, u8 drawMode, u8 highlightEnabled);
void advancePanelScroll(void);
void seedMenuScratchAndSetWorkByteBWorld00Area03004(void);
void advancePanelStepClaimEntryAndDrawCountValueWorld00Area03004_801D6F34(void);
void seedMenuScratch(void);
void clampMarkerPositionsAndDrawPanelRowWorld00Area03004(u8 cellVariant);
void stepPanelMotionScriptAndDrawMarkerRowWorld00Area03004(void);
void dispatchStateByte3Table20c4World00Area03004(void);
void advanceStepMode6(void);
void clearFlagState10(void);
void adjustAndClampCurrentWorkRangeValueWorld00Area03004(s8 delta);
void seedSharedValuesAndAdvanceWorkStepIfGateClearWorld00Area03004(void);
void dispatchCompanion(void);
void dispatchStateByte2Table2340World00Area03004(void);
void dispatchHandlerBySelectorByteWorld00Area03004(void);
void dispatchStateByte4Table22f0World00Area03004(void);
void advanceWorkStepOnStateThreeOrFourAndDrawTextWorld00Area03004(void); /* AREA030 handler-table entry 0 at 0x801E22F0 */
/* Same-binary grouped-entry quantity updater at 0x801650B4 (executable space): the AREA030
 * panel step passes (0, active record byte 6 + 0x38, 1, 0) and branches on the
 * byte-valued result. Body not lifted in this target. */
u8 addGroupedEntryQuantity(u8 arg0, u8 arg1, u8 arg2, u8 arg3); /* @source 0x801650B4 */
/* Same-binary panel position submitter at 0x801647C4 (executable space): the
 * AREA030 flagged handler passes the panel id 0x1C and the masked flag byte
 * scaled by 16 plus 0x58 as an unsigned halfword pair, and discards the result. */
void func_801647C4(u16 arg0, u16 arg1, s32 arg2);
/* Shared equipment icon helper at 0x800F7094. AREA030 passes signed screen
 * coordinates, the equipment-index byte, 3 and a selection flag of 0 or 1.
 * The callee body and payload owner are not established here. */
void func_800F7094(s16 arg0, s16 arg1, s32 arg2, s32 arg3, s32 arg4);
/* SLUS text-run drawer at 0x8014FC90. Coordinates are passed as words and
 * narrowed to signed halfwords before glyph emission. Attributes and the run
 * limit use their low bytes. Returns one byte past its final input cursor;
 * AREA030 discards the result. */
const u8* func_8014FC90(s32 x, s32 y, s32 attributes, u32 limit,
                       const u8* input);
void saveAndUpdateTextAdvanceStateWorld00Area03004(void);
void updateRecordCursorsFromNonzeroIndicesWorld00Area03004(void);
void storeRequestedStateAndAdvanceSelectedModesWorld00Area03004(s32 state);
void updateScratchMarkerAndCountdownFromPadFlagsWorld00Area03004(void);
void playCueForChangedSelectedValueWorld00Area03004(void);
void updateFourTextSlotsAndPublishCompletedSlotLimitWorld00Area03004(void);
void func_801DDDBC(void);
void dispatchStateByte3Table22a4World00Area03004(void);
void dispatchStateByte3Table22b0World00Area03004(void);
void dispatchStateByte3Table22bcWorld00Area03004(void);
void dispatchStateByte3Table22c8World00Area03004(void);
void initializeWorkDispatchBytesIfGateClearWorld00Area03004(void);
void clearRecordHeadFieldsAndSetStateTwoWorld00Area03004(void);
void seedChannelPairAndAdvanceWorkDispatchWorld00Area03004_801DC74C(void);
void incrementSharedCounterAndSetWorkStepWorld00Area03004(void);
void applyEntryPadConfirmOrCancelWorld00Area03004_801DA638(void);
void dispatchStateByte3Table2258World00Area03004(void);
void initializeWorkDispatchBytesIfBothGatesClearWorld00Area03004(void);
void dispatchStateByte3Table2268World00Area03004(void);
/* Draw the selected entry label and amount summary with the caller's slide
 * count. The second argument is the displayed amount, including zero. */
void drawSlidingEntryLabelAndAmountSummaryWorld00Area03004(s32 entryId, s32 amount, s32 slideCount);
/* Target-local extra-icon drawer at 0x801E1C7C, gated by the D_801E3214 flag
 * byte and bit 3 of the shared frame counter D_80143E6C. */
void drawExtraIconWhenEnabledAndFrameBitThreeSetWorld00Area03004(void);
/* Test the original shared gates and record predicate. Success leaves the
 * handler unchanged; failure writes 1 to work handler byte 1. */
u8 checkWorkEntryGateOrSelectHandlerOneWorld00Area03004(void);
/* Target-local AREA030 step derivation at 0x801E03F8: rebuilds the record step
 * words from the route index byte 0x08. */
void deriveAndReflectWorkRouteStepsThenReindexWorld00Area03004(void);
/* Target-local AREA030 step integrator at 0x801E0394: while the record
 * countdown byte 0x09 is non-zero it decrements it, adds the step words into
 * the record position words and the countdown step word into the timer. */
void applyWorkStepTripleWhileCountdownActiveWorld00Area03004(void);
/* Target-local AREA030 staging-window probe at 0x801E0770: reports whether the
 * record published at the scratchpad cursor 0x1F800044 sits inside the shared
 * reference window without arming anything. */
u8 checkWorkReferenceBoundsUnderSharedGatesWorld00Area03004(void);
void submitPanelPair(s16 arg0, s16 arg1);
void configureSpriteClut(s16 x, s16 y, u8 packedSelector);
void appendDimTile(void);

/* cursor and mask are distinct simple output locals. */
#define WORLD00_AREA030_DRAW_HEADER_AND_STATUS(cursor, mask) \
  (drawFilledPanelWithInsetBorderWorld00Area03004(20, 18, 280, 19, 0), \
   (mask) = 0u, \
   (D_80145E93 >= 3u \
        ? ((cursor) = D_1F800044, \
           (mask) = (u8)(1u << (cursor)[6])) \
        : 0u), \
   drawLabeledStatusCellsWorld00Area03004_801D7C10(88, 40, (mask)))

/* Preserve the repeated record reads. Evaluate icon_row_y once, at the final
 * call. */
#define WORLD00_AREA030_DRAW_PANEL_ROW(icon_row_y)                           \
  (submitPanelPair(0, 78),                                                   \
   configureSpriteClut(0x80, 91, D_80146888[D_801E3208].unk_02[4]),          \
   drawSlidingEntryLabelAndAmountSummaryWorld00Area03004(D_80146888[D_801E3208].unk_02[4],                           \
                 D_80146888[D_801E3208].unk_90[0], 0),                       \
   drawSlotValueSummaryAndThresholdIconsWorld00Area03004(176, (icon_row_y)))

/* work_cell is read five times. Pass a volatile pointer-cell lvalue and distinct
 * simple cursor locals; each halfword value is evaluated once. */
#define WORLD00_AREA030_INIT_PANEL_WORK(work_cell, halfword_cursor,           \
                                      next_cursor, halfword_2e, halfword_30)  \
  do {                                                                        \
    applyMainLookupRecord(0x47);                                                      \
    (work_cell)[0x24] = 0x80;                                                 \
    (work_cell)[0x29] = 1;                                                    \
    (work_cell)[0x2a] = 0;                                                    \
    (halfword_cursor) = (work_cell);                                          \
    (halfword_cursor)[0x48] = 0;                                              \
    (next_cursor) = (work_cell);                                              \
    *(volatile u16*)((halfword_cursor) + 0x2e) = (halfword_2e);               \
    *(volatile u16*)((halfword_cursor) + 0x30) = (halfword_30);               \
  } while (0)

/* first is a captured cursor; cell is reread for each later store. */
#define WORLD00_AREA030_CLEAR_PANEL_LEVELS_AND_COUNTERS(first, cell) \
  ((first)[0x5D] = 0, (cell)[0x5E] = 0, (cell)[0x5F] = 0, \
   (cell)[9] = 0, (cell)[10] = 0)

/* Arm an entry-panel record. Pass a stable unsigned slot local, read three times. */
#define WORLD00_AREA030_ARM_PANEL_ENTRY(slot) \
  (D_80143FC8[(slot)].flags_00 = 1, \
   D_80143FC8[(slot)].byte_05 = 0x1A, \
   D_80143FC8[(slot)].unk_01 = 0x10)

#define WORLD00_AREA030_SCRATCH_PTR PSX_REF(volatile u8*, 0x1f800044u)


#define WORLD00_AREA030_DISPATCH_SLOT_TWO(name, table)                         \
  void NO_SIBLING_CALLS name(void) {                                           \
    table[D_1F800044[2]]();                                                    \
  }

#endif
