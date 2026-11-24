#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

/* Work record published at the shared scratchpad pointer cell 0x1F800044,
 * viewed at the offsets this drawer touches: the first-frame byte at 0x02, the
 * layer selector byte at 0x0B, the projected position words at 0x34 and 0x38.
 * The two screen halfwords at 0x36 and 0x3A are reached through FIELD_REF so no
 * struct field claims a role the bytes do not show. */
typedef struct World01Area06513PanelWork {
  u8 unk_00[2];
  u8 unk_02;
  u8 unk_03[8];
  u8 unk_0b;
  u8 unk_0c[0x28];
  s32 unk_34;
  s32 unk_38;
} World01Area06513PanelWork;

/* Work record of the current overlay step, bound under this target's own
 * spelling for the scratchpad pointer cell. */
/* @source 0x1F800044 @kind data */
extern World01Area06513PanelWork* stateWorkCursor;

/* Scratchpad block at 0x1F800014 holding the four vertices of the projected
 * panel. */
/* @source 0x1F800014 @kind data */
extern SVECTOR D_1F800014[];

/* Shared frame counter: its low nibble advances the halfword at 0x3A and its
 * low five bits set the panel pulse. */
/* @source 0x80143E6C @kind data */
extern s32 D_80143E6C;

/* Shared gate byte whose bit 4 keeps the drawer active. */
/* @source 0x8014832E @kind data */
extern u8 D_8014832E;

/* Shared map height byte the halfword at 0x3A wraps against. */
/* @source 0x80104001 @kind data */
extern u8 D_80104001;

/* Shared focus halfwords the work halfwords at 0x36 and 0x3A are compared
 * with. */
/* @source 0x8014930A @kind data */
extern s16 D_8014930A;
/* @source 0x8014930E @kind data */
extern s16 D_8014930E;

/* Trailing byte tables of this payload, each indexed by work byte 0x0B minus
 * two: 0x801F534C holds the cell's u origin, 0x801F5350 its v origin and
 * 0x801F5354 the cell extent, which the two reachable layers read as 0x0A and
 * 0x0C. */
/* @source 0x801F534C @kind data */
extern u8 D_801F534C[];
/* @source 0x801F5350 @kind data */
extern u8 D_801F5350[];
/* @source 0x801F5354 @kind data */
extern u8 D_801F5354[];

/* Shared command helper that stamps the layer byte into the built packet. */
/* @source 0x80155560 */
extern void func_80155560(u32 command, void* primitive, s32 mode);
/* Shared cell lookup returning the projected record for a grid coordinate. */
/* @source 0x801559AC */
extern void* func_801559AC(s32 column, s32 row);
/* Shared cell queue taking both grid coordinates shifted left by sixteen. */
/* @source 0x80155A08 */
extern void func_80155A08(s32 column, s32 row, s32 mode, s32 size);

/* Shared graphics-mode probe that selects the cell tpage value. */
/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* @source 0x801F47A0
 * @behavior Draws the pulsing textured panel of the focus area and then the
 *           scrolling grid of semitransparent textured cells around it.
 *           Initializes work byte 0x02 and adds the low nibble of the shared
 *           frame counter 0x80143E6C to work halfword 0x3A, returns while bit 4
 *           of the shared byte 0x8014832E is clear, advances work word 0x38 by
 *           0x800 plus work byte 0x0B minus two shifted left ten, wraps
 *           halfword 0x3A to -8 above the map height byte 0x80104001 plus
 *           eight, and returns unless a shared focus halfword 0x8014930A or
 *           0x8014930E lies within 0x1A of work halfword 0x36 or 0x3A. It then
 *           projects one textured quad at depth -0x300 over the scratchpad
 *           vectors 0x1F800014 through RotTransPers4 with a half extent of the
 *           layer width plus the frame counter low five bits mirrored around
 *           0xF, hands it to the shared 0x80155560 with the layer byte out of
 *           0xBB509100 and reserves it through GpuAppendPrim(4, 0x28), then
 *           walks a (2 - layer) * 4 + 1 by (2 - layer) * 4 grid whose cells
 *           come from the shared 0x801559AC lookup and take their texture
 *           window from this payload's trailing byte tables 0x801F534C,
 *           0x801F5350 and 0x801F5354 at index layer before being queued
 *           through the shared 0x80155A08 with both coordinates shifted left by
 *           16. Takes no arguments and returns nothing.
 * @status partial
 * @match 83.03
 * @residual 323 of 389 instructions at 1556 bytes, frame and byte count equal
 *           to the original. First difference at +0x008C: the original loads
 *           the byte at 0x0B into $v1, forms the layer with `addiu s2,v1,-2`,
 *           adds 0x800 to the word at 0x38 and shifts the layer itself with
 *           `sll v1,s2,0xa`, while this build loads the byte into $s3, shifts
 *           the unadjusted byte and cancels the +0x800 against the -2 that the
 *           signed reassociation of `(layer << 10)` folds into the sum, then
 *           sinks `addiu s2,s3,-2` into the delay slot of the clamp branch.
 *           The layer still lands in $s2, so the rest of the 66 differences are
 *           the schedule and the $t0/$t1/$t2 permutation that follow from that
 *           fold. Seven clean-C shapes were measured in this lane and all came
 *           back at 323/389 or worse: a named `u8` byte temp, a separate shifted
 *           term temp, the multiplication form `layer * 0x400`, and moving
 *           `layer` first among the locals each reproduced 323/389 at +0x008C;
 *           computing `half`, `x` and `y` before the four scratchpad stores was
 *           neutral at the same 323/389, and both the inlined
 *           `((scratch->unk_0b - 2) << 10)` form and the same with `half` moved
 *           ahead of `vertices = D_1F800014` regressed to 302/390 at 1560 bytes
 *           with the first difference at +0x0008. This body is the family one:
 *           its 0x614 bytes compare equal to emi/world02/area088/13 0x801F52C8
 *           apart from sixteen `lbu` %lo displacements that name this payload's
 *           own three trailing byte tables 0x801F534C, 0x801F5350 and
 *           0x801F5354, so every other word including each `jal` and branch
 *           encoding is identical; the same body measures 323/389 in that target
 *           and in emi/world00/area016/13, emi/world00/area033/13,
 *           emi/world03/area151/13 and emi/world04/area152/13. Smallest missing
 *           evidence: the original allocation and schedule of `layer` across the
 *           scroll and clamp block, which this source form cannot reach; the
 *           recorded flag search over 52 profiles in the family is negative.
 */
void renderPulsingPanel(
    void) {
  volatile World01Area06513PanelWork* first_frame;
  World01Area06513PanelWork* scratch;
  u32 primitive;
  SVECTOR* vertices;
  void* record;
  s32 layer;
  s32 span;
  s32 row;
  s32 column;
  s32 half;
  s32 x;
  s32 y;
  s32 tpage;
  long depth;
  long flag;

  first_frame = stateWorkCursor;
  if (first_frame->unk_02 == 0) {
    u8 count;
    u8 next;

    count = first_frame->unk_02;
    next = count + 1;
    FIELD_REF(u16, first_frame, 0x3au) += D_80143E6C & 0xf;
    first_frame->unk_02 = next;
  }
  if ((D_8014832E & 4) == 0) {
    return;
  }

  scratch = stateWorkCursor;
  layer = (s32)scratch->unk_0b - 2;
  scratch->unk_38 = scratch->unk_38 + 0x800 + (layer << 10);
  if (D_80104001 + 8 < FIELD_REF(s16, scratch, 0x3au)) {
    FIELD_REF(s16, scratch, 0x3au) = -8;
  }

  scratch = stateWorkCursor;
  if (abs(D_8014930A - FIELD_REF(s16, scratch, 0x36u)) >= 0x1a &&
      abs(D_8014930E - FIELD_REF(s16, scratch, 0x3au)) >= 0x1a) {
    return;
  }

  primitive = (u32)g_PrimCursor;
  row = 0;
  SetPolyFT4((POLY_FT4*)primitive);
  SetShadeTex((POLY_FT4*)primitive, 0);

  vertices = D_1F800014;
  SPAD_REF(s16, 0x18u) = -0x300;
  SPAD_REF(s16, 0x20u) = -0x300;
  SPAD_REF(s16, 0x28u) = -0x300;
  SPAD_REF(s16, 0x30u) = -0x300;

  half = ((2 - layer) * 256) + abs(0xf - (D_80143E6C & 0x1f));
  x = (stateWorkCursor->unk_34 >> 9) - 0x3fc0;
  y = (stateWorkCursor->unk_38 >> 9) - 0x3fc0;

  vertices[0].vx = x - half;
  SPAD_REF(s16, 0x16u) = y - half;
  vertices[1].vx = x + half;
  SPAD_REF(s16, 0x1eu) = y - half;
  vertices[2].vx = x - half;
  SPAD_REF(s16, 0x26u) = y + half;
  vertices[3].vx = x + half;
  SPAD_REF(s16, 0x2eu) = y + half;

  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)(primitive + 8), (long*)(primitive + 0x10),
                (long*)(primitive + 0x18), (long*)(primitive + 0x20), &depth,
                &flag);

  func_80155560(layer | 0xbb509100u, (void*)primitive, 1);
  GpuAppendPrim(4, 0x28);

  span = (2 - layer) * 4;
  for (row = 0; row <= span; row++) {
    if (span <= 0) {
      continue;
    }
    for (column = 0; column < (2 - layer) * 4; column++) {
      record = func_801559AC(
          FIELD_REF(s16, stateWorkCursor, 0x36u) - 2 + layer + column,
          FIELD_REF(s16, stateWorkCursor, 0x3au) - 2 + layer + row);
      if (record == 0) {
        continue;
      }

      primitive = (u32)g_PrimCursor;
      SetPolyFT4((POLY_FT4*)primitive);
      SetShadeTex((POLY_FT4*)primitive, 0);
      SetSemiTrans((POLY_FT4*)primitive, 1);

      *(u32*)(primitive + 8) = *(u32*)((u8*)record + 8);
      *(u32*)(primitive + 0x10) = *(u32*)((u8*)record + 0x10);
      *(u32*)(primitive + 0x18) = *(u32*)((u8*)record + 0x18);
      *(u32*)(primitive + 0x20) = *(u32*)((u8*)record + 0x20);
      *(u16*)(primitive + 0x0e) = 0x78cb;

      if (GetGraphType() == 1) {
        tpage = 0x12b;
      } else if (GetGraphType() == 2) {
        tpage = 0x12b;
      } else {
        tpage = 0x5b;
      }
      *(u16*)(primitive + 0x16) = tpage;

      *(u8*)(primitive + 4) = 0x28;
      *(u8*)(primitive + 5) = 0x28;
      *(u8*)(primitive + 6) = 0x28;

      *(u8*)(primitive + 0x0c) = D_801F534C[layer] + column * D_801F5354[layer];
      *(u8*)(primitive + 0x0d) =
          D_801F5350[layer] + row * D_801F5354[layer] -
          (D_801F5354[layer] *
           FIELD_REF(u16, stateWorkCursor, 0x38u)) / 0x10000;
      *(u8*)(primitive + 0x14) =
          D_801F5354[layer] +
          (D_801F534C[layer] + column * D_801F5354[layer]);
      *(u8*)(primitive + 0x15) =
          D_801F5350[layer] + row * D_801F5354[layer] -
          (D_801F5354[layer] *
           FIELD_REF(u16, stateWorkCursor, 0x38u)) / 0x10000;
      *(u8*)(primitive + 0x1c) = D_801F534C[layer] + column * D_801F5354[layer];
      *(u8*)(primitive + 0x1d) =
          D_801F5354[layer] +
          (D_801F5350[layer] + row * D_801F5354[layer] -
           (D_801F5354[layer] *
            FIELD_REF(u16, stateWorkCursor, 0x38u)) / 0x10000);
      *(u8*)(primitive + 0x24) =
          D_801F5354[layer] +
          (D_801F534C[layer] + column * D_801F5354[layer]);
      *(u8*)(primitive + 0x25) =
          D_801F5354[layer] +
          (D_801F5350[layer] + row * D_801F5354[layer] -
           (D_801F5354[layer] *
            FIELD_REF(u16, stateWorkCursor, 0x38u)) / 0x10000);

      func_80155A08(
          (FIELD_REF(s16, stateWorkCursor, 0x36u) - 2 + layer +
           column) << 16,
          (FIELD_REF(s16, stateWorkCursor, 0x3au) - 2 + layer +
           row) << 16, 1, 0x28);
    }
  }
}
