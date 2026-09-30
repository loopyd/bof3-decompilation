#ifndef EMI_WORLD00_AREA032_13_INTERNAL_H
#define EMI_WORLD00_AREA032_13_INTERNAL_H

#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Entry table at 0x80143FC8 — 0x74 bytes per record. Only the occupancy byte
 * at 0x00, the bytes at 0x05, 0x06 and 0x09, the work-record index at 0x0B and
 * the copied cursor words at 0x34/0x38/0x3C are proven for this target. */
typedef struct World00Area032Record {
  u8 flags_00;
  u8 unk_01;
  u8 unk_02;
  u8 unk_03;
  u8 unk_04;
  u8 unk_05;
  u8 unk_06;       /* copied from cursor byte 0x08 */
  u8 pad_07[2];    /* 0x07 - 0x08 */
  u8 unk_09;
  u8 pad_0A;       /* 0x0A */
  u8 unk_0B;       /* index of the current work record at 0x80146888 */
  u8 pad_0C[0x28]; /* 0x0C - 0x33 */
  s32 unk_34;
  s32 unk_38;
  s32 unk_3C;
  u8 pad_40[0x34]; /* 0x40 - 0x73 */
} World00Area032Record;

/* Shared work records at 0x80146888; 0x80146884 publishes the pointer to the
 * current one. Proven for this target: the 0x98-byte element size and the
 * flag byte at 0x00, the signed 0x34/0x38 accumulators and the 0x5C byte that
 * func_801F30EC touches on the two records it selects. */
typedef struct World00Area032WorkRecord {
  u8  flags_00;
  u8  pad_01[0x33]; /* 0x01 - 0x33 */
  s32 unk_34;
  s32 unk_38;
  u8  pad_3C[0x20]; /* 0x3C - 0x5B */
  u8  unk_5C;
  u8  pad_5D[0x3B]; /* 0x5D - 0x97 */
} World00Area032WorkRecord;

/* @source 0x1F800000 @kind unknown — signed work-record slot index that
 * func_801A4D54 resolves into the 0x80146888 table and publishes at
 * 0x80146884; func_801F30EC stores the two selected cursor ids there. */
extern u16 D_1F800000;
/* @source 0x1F800014 @kind unknown */
extern s16 D_1F800014;
/* @source 0x1F800016 @kind unknown */
extern s16 D_1F800016;
/* @source 0x1F800018 @kind unknown */
extern s16 D_1F800018;
/* @source 0x1F800034 @kind unknown */
extern u16 D_1F800034;
/* @source 0x1F800036 @kind unknown */
extern u16 D_1F800036;
/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;
/* @source 0x80143FC8 @kind unknown */
extern World00Area032Record D_80143FC8[];
/* @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;
/* @source 0x801448ED @kind unknown */
extern u8 D_801448ED;
/* @source 0x80145E90 @kind unknown — work base published as the scratchpad
 * cursor by func_801F363C. */
extern u8 D_80145E90[];
/* @source 0x80145EC4 @kind unknown */
extern s32 D_80145EC4;
/* @source 0x80145EC8 @kind unknown */
extern s32 D_80145EC8;
/* @source 0x80145ECE @kind unknown */
extern s16 D_80145ECE;
/* @source 0x80146884 @kind unknown */
extern u8* D_80146884;
/* @source 0x80146888 @kind unknown */
extern u8 D_80146888[];
/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x801490A8 @kind unknown */
extern u16 D_801490A8;
/* @source 0x801490C7 @kind unknown */
extern s8 D_801490C7;
/* @source 0x801F3F6C @kind table — handler table selected by scratch cursor
 * byte 1 at 0x801F2C04; the adjacent table at 0x801F3F80 is the one byte 1
 * selects at 0x801F30A8. */
extern void (*D_801F3F6C[])(void);
/* @source 0x801F3F80 @kind table */
extern void (*D_801F3F80[])(void);
/* @source 0x801F3F8C @kind unknown — local data script that func_801F30EC
 * hands to func_801A4D54 while activating the two selected work records. */
extern u8 D_801F3F8C[];
/* @source 0x801F4900 @kind table */
extern void (*D_801F4900[])(void);
/* @source 0x801F4908 @kind table */
extern void (*D_801F4908[])(void);

void func_80196070(void);

/* @source 0x8015CA48 — returns the identifier of one free local effect slot;
 * func_801F30EC stores two of them at cursor offsets 3 and 4 and treats 0xFF
 * as "none left". */
u8 func_8015CA48(void);

/* @source 0x801A4D54 — activates one 0x80146888 work record from the signed
 * slot index at D_1F800000 and the data script handed in arg0, publishing the
 * record at 0x80146884 and as the scratch cursor at 0x1F800044. */
void func_801A4D54(void* arg0);

/* Shared work-record submitter; func_801F35B4 hands it the 0x98-byte record of
 * D_80146888 selected by a scratch cursor byte. */
void func_80196718(void* arg0);

/* Shared work-record effect submitter; func_801F3920 hands it the effect byte at
 * 0x93 of the current work record at 0x80146884. Every caller loads that byte
 * with lbu, and the engine-side caller masks it with andi 0xFF, so the
 * parameter is one byte wide. */
void func_80196670(u8 arg0);

u8 func_8019601C(void);

/* Positional-effect submitter; the caller-visible byte result and the five
 * arguments are fixed by the call sites at 0x801F3844 and 0x801DDAB4. The
 * callee body is not in the SLUS image (file offset 0xFFD1C is zero-filled). */
s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_8017A97C(void* arg0);
void func_801C2438(void);

/* Cursor level-state query: the argument is the signed level delta applied to
 * the 0x5D/0x5E/0x5F triple of the scratch cursor record at 0x1F800044; the
 * result is the 0/1 range flag compared by the caller at 0x801F378C. */
s32 func_801C29A0(s32 arg0);
void func_8014D6B8(u32 flag);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x8015DF18 @kind unknown — shared frontend cue dispatcher called by
 * func_801F2DAC with cue id 0x203 while scratch work byte 9 reads 0xD7. */
void func_8015DF18(u16 arg0);

void func_801F2F04(s16 arg0, s16 arg1, s16 arg2, u8 arg3, u8 arg4);

#endif
