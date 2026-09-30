#ifndef EMI_WORLD00_AREA024_14_INTERNAL_H
#define EMI_WORLD00_AREA024_14_INTERNAL_H

#include <rand.h>

#include "bof3/bof3.h"

typedef void (*World00Area024Handler)(void);

typedef struct World00Area024SpriteWork {
  u8     field_00;
  u8     field_01;
  u8     field_02;
  u8     field_03;
  s32    field_04;
  s32    field_08;
  s32    field_0c;
  u8     unk_10[4];
  VECTOR field_14;
  u16    field_24;
} World00Area024SpriteWork;

typedef struct World00Area024Scratch {
  u8  unk_00;
  u8  mode;
  u8  unk_02[0x32];
  s32 field_34;
  s32 field_38;
  s32 field_3c;
} World00Area024Scratch;

typedef struct World00Area024SpinWork {
  s32     field_00;
  s32     field_04;
  s32     field_08;
  u8      unk_0c[4];
  SVECTOR field_10;
  SVECTOR field_18;
  s16     field_20;
  s16     field_22;
  s16     field_24;
  s16     unk_26;
  s16     field_28;
  s16     field_2a;
} World00Area024SpinWork;

/* One 6-byte quad corner: three local halfword coordinates. */
typedef struct World00Area024Vertex {
  u16 x;
  u16 y;
  u16 z;
} World00Area024Vertex;

/* One 0x28-byte local quad record of the 27-entry tables at 0x800e4bc8: a
 * leading halfword followed by four 6-byte corners, then 0x0e trailing bytes. */
typedef struct World00Area024Quad {
  u16                  field_00;
  World00Area024Vertex vertex[4];
  u8                   unk_1a[0x0e];
} World00Area024Quad;

/* One 0x18-byte local motion record of the 27-entry table at 0x800e4940: three
 * position halfwords, three velocity halfwords and three rotation halfwords,
 * each group separated by one unwritten halfword. */
typedef struct World00Area024Motion {
  u16 x;
  u16 y;
  u16 z;
  u16 unk_06;
  u16 vx;
  u16 vy;
  u16 vz;
  u16 unk_0e;
  s16 rx;
  s16 ry;
  s16 rz;
  s16 unk_16;
} World00Area024Motion;

/* One 8-byte screen-projected point of the projection workspace used by the
 * sprite-work drawer func_801F2DF8: the shared projection helper func_801AFF04
 * writes the x/y halfword pair at +0x00/+0x02, and the drawer reads that pair
 * plus the half-size pair handed to func_801AFFD8 back at +0x08/+0x0a. The
 * eight-byte stride and the four-entry workspace are proven by that function:
 * its address-taken local occupies +0x10..+0x2f of the original 0x40-byte
 * frame, while only its first two entries are read. This overlay only proves
 * the x/y halfwords. */
typedef struct World00Area024ScreenPoint {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} World00Area024ScreenPoint;

/* @source 0x1F800044
 * @kind unknown — scratchpad pointer cell: current overlay work record. */
extern u8*                             D_1F800044;
/* @source 0x80147A58
 * @kind unknown — raw fixed-RAM byte; cleared by the entry-3 handler. */
extern volatile u8  D_80147A58;
/* @source 0x801490A8
 * @kind unknown — raw fixed-RAM halfword; set to -1 by both state-flag
 * reset callbacks. */
extern u16          D_801490A8;
/* @source 0x801F5B00
 * @kind bss — current 0x28-byte work entry cursor; seeded from the work base
 * and walked by the dispatcher. */
extern u8*                              workCursor;
/* @source 0x801F2C04
 * @kind rodata — this overlay's zeroed 16-byte vector at the head of the
 * T_801F2C00 rodata segment; copied into the local transform scratch by the
 * spin-line drawer. */
extern VECTOR                           D_801F2C04;
/* @source 0x801F4214
 * @kind rodata — four-entry per-mode handler table dispatched by the work
 * dispatcher on each active entry's byte +0x01. */
extern const World00Area024Handler      stateTable[];
/* @source 0x801F4200 @kind unknown */
extern World00Area024Handler            D_801F4200[];
/* shared frame counter; owned by the shared map (0x80143E6C) and read as the
 * frame-parity bit by the motion integrator. */
extern s32                              D_80143E6C;

void func_8015B410(void* arg0);
void func_8015B4B0(void* arg0);
void func_80196070(void);
void func_801AFF64(void* arg0);
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);
u16  func_8017A6F0(s32 arg0, s32 arg1);
u16  func_8017A620(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8017A97C(void* arg0);
void func_8017A904(void* arg0, s32 arg1);
void func_8017A9B8(void* arg0);
void func_8017AAE8(void* arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801782FC — shared angular helper; the spin triangle rotates its
 * local halfwords through it. */
s32 func_801782FC(s32 arg0);
/* @source 0x801783C8 — companion helper for the first term of the same two
 * spin-triangle rotations. */
s32 func_801783C8(s32 arg0);

/* @source 0x80179C58 — shared rotation helper applied to the spin-work
 * initializer's scratch matrix with its first masked rand() angle. */
void func_80179C58(s32 arg0, MATRIX* arg1);
/* @source 0x80179DF8 — companion rotation helper for the same scratch matrix,
 * applied with the negated second masked rand() angle. */
void func_80179DF8(s32 arg0, MATRIX* arg1);
/* @source 0x80179F98 — companion rotation helper for the same scratch matrix,
 * applied with the third masked rand() angle. */
void func_80179F98(s32 arg0, MATRIX* arg1);

/* @source 0x8015DF18 @kind unknown — shared frontend cue dispatcher called by
 * the overlay step func_801F2CB0 with cue ids 0x206/0x20E/0x203. */
void func_8015DF18(u16 arg0);

void func_801F2DF8(const void* arg0);
void func_801F2FD4(void* arg0);
void func_801F3080(void);
void func_801F3314(void);
/* @source 0x80178804 @kind unknown — shared rotation helper called by the
 * motion initializer with the averaged quad position and the derived velocity
 * halfwords. */
void func_80178804(VECTOR* arg0, SVECTOR* arg1);
/* @source 0x80178818 @kind unknown — shared helper called in place on one work
 * entry's vector by the per-entry initializer func_801F2FD4. */
void func_80178818(void* arg0, void* arg1);
s32  dispatchWorkStates(void);
void advanceWorkStateWithDoubleVelocity(void);
void advanceWorkStateUntilExpired(void);
void func_801F362C(void);
void func_801F3708(void* arg0, void* arg1, s16* arg2);
void func_801F3944(World00Area024SpinWork* arg0);
void func_801F3BE4(void* arg0);
void initSpinWork(void);
void drawSpin(void);
s32  func_801F3E48(u8 arg0);
s16  func_801F4158(const u16* arg0, const u16* arg1, const u16* arg2);

#define WORLD00_AREA024_PRIMITIVE_PTR  PSX_REF(volatile u8*, 0x8014598cu)
#define WORLD00_AREA024_WORK_PTR       PSX_REF(volatile u8*, 0x801f5b00u)
#define WORLD00_AREA024_WORK_BASE      PSX_PTR(u8, 0x800e4800u)
#define WORLD00_AREA024_SPIN_WORK_BASE PSX_PTR(u8, 0x800e5000u)
/* 27-entry array of 0x28-byte target records consumed by the motion
 * integrator; it ends where the spin-work table begins. */
#define WORLD00_AREA024_TARGET_BASE    PSX_PTR(u8, 0x800e4bc8u)
/* Pointer cell holding the matching 27-entry array of 0x28-byte source
 * records; loaded once per call, not per record. */
#define WORLD00_AREA024_SOURCE_TABLE   PSX_REF(u8*, 0x80147aa8u)
/* Pointer cell whose leading signed byte is the number of 0x28-byte quad
 * records the spin-line drawer walks; read once before its loop. */
#define WORLD00_AREA024_RECORD_LIST    PSX_REF(u8*, 0x80147aacu)
/* Base of the 27-entry motion table: each 0x18-byte record holds three
 * position halfwords at +0x00, +0x02 and +0x04, three matching velocity
 * halfwords at +0x08, +0x0a and +0x0c, and the three rotation halfwords at
 * +0x10, +0x12 and +0x14 that func_801F3708 refills. */
#define WORLD00_AREA024_MOTION_BASE    PSX_PTR(u16, 0x800e4940u)
#define WORLD00_AREA024_SCRATCH_PTR                                            \
  PSX_REF(volatile World00Area024Scratch*, 0x1f800044u)

#endif
