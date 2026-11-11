#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

/* Work record published at the scratchpad pointer cell 0x1F800044. The word at
 * 0x34 and the word at 0x38 are each read as a word for the projected position
 * and again as the halfword two bytes in for the grid coordinate, so those two
 * halfwords stay field access over the word members. */
typedef struct Area15213PanelWork {
  u8 unk_00[2];
  u8 unk_02;
  u8 unk_03[8];
  u8 unk_0b;
  u8 unk_0c[0x28];
  s32 unk_34;
  s32 unk_38;
} Area15213PanelWork;

/* Scratchpad cell publishing this overlay's work record. */
/* @source 0x1F800044 @kind unknown */
extern Area15213PanelWork* area15213WorkCursor_1F800044;

/* Shared distance-gated frame counter and gate byte. */
/* @source 0x80143E6C @kind data */
extern s32 D_80143E6C;
/* @source 0x8014832E @kind data */
extern u8 D_8014832E;
/* @source 0x80104001 @kind data */
extern u8 D_80104001;
/* @source 0x8014930A @kind data */
extern s16 D_8014930A;
/* @source 0x8014930E @kind data */
extern s16 D_8014930E;

/* This payload's four projected vertices. */
/* @source 0x1F800014 @kind data */
extern SVECTOR D_1F800014[];

/* This payload's trailing cell tables, each indexed by work byte 0x0B minus
 * two: the base texture coordinate in u, the base coordinate in v and the cell
 * extent. */
/* @source 0x801F4E30 @kind data */
extern u8 D_801F4E30[];
/* @source 0x801F4E34 @kind data */
extern u8 D_801F4E34[];
/* @source 0x801F4E38 @kind data */
extern u8 D_801F4E38[];

/* @source 0x80155560 */
extern void func_80155560(u32 command, void* primitive, s32 mode);
/* @source 0x801559AC */
extern void* func_801559AC(s32 column, s32 row);
/* @source 0x80155A08 */
extern void func_80155A08(s32 column, s32 row, s32 mode, s32 size);

/* @source 0x801F4628
 * @behavior Initializes work byte 0x02 and adds the low nibble of the shared
 *           frame counter 0x80143E6C to work halfword 0x3A. Returns while bit 4
 *           of the shared byte 0x8014832E is clear, then advances work word
 *           0x38 by 0x800 plus the layer value shifted left ten, where the
 *           layer is work byte 0x0B minus two, and wraps work halfword 0x3A to
 *           -8 once it exceeds the map height byte 0x80104001 plus eight.
 *           Returns again unless either shared focus halfword 0x8014930A or
 *           0x8014930E lies within 0x1A of work halfword 0x36 or 0x3A. It then
 *           projects one textured quad at depth -0x300 whose half extent is the
 *           layer-derived width plus the shared counter low five bits mirrored
 *           around 0xF, opens that primitive with the shared append helper
 *           0x8014E5A0 and draws a grid of semitransparent textured cells from
 *           the shared lookup helper 0x801559AC. The two loops run over the
 *           layer-derived count and take their texture coordinates from this
 *           payload's three trailing cell tables 0x801F4E30, 0x801F4E34 and
 *           0x801F4E38, the last of which also scales the work halfword at 0x38
 *           through a signed fixed-point divide by 0x10000. Each cell closes
 *           with the shared helper 0x80155A08 passed the grid coordinate pair
 *           shifted left sixteen. Takes no arguments and returns nothing. The
 *           0x614 payload bytes compared equal to the same body in
 *           emi/world00/area033/13 at 0x801F469C and in six further overlays
 *           apart from sixteen words, each one the %lo displacement of this
 *           payload's own trailing byte tables 0x801F4E30, 0x801F4E34 and
 *           0x801F4E38, so the body follows that family's statement order with
 *           this target's table addresses.
 * @status partial
 * @match 83.03
 * @residual 323/389 instructions at 1556 bytes, frame and byte count equal to
 *           the original. The first difference at +0x8C is the register of the
 *           `lbu` of work byte 0x0B, v1 in the original against s3 here, and the
 *           addiu of 0x800 missing from the scroll update: this build folds
 *           0x800 + ((byte - 2) << 10) into byte << 10 and then sinks the layer
 *           definition into the following branch delay slot. Twelve measured
 *           probes of that one statement class - split reduction, shift-first
 *           operand order, named scroll and step intermediates, uncast and
 *           unsigned variants and the parenthesised spelling - either
 *           reproduced this residual or spent an extra saved register and four
 *           frame bytes.
 */
void drawPulsingTexturedPanelAndScrollingCellGridNearFocusWorld04Area15213_801F4628(void) {
  volatile Area15213PanelWork* first;
  Area15213PanelWork* scratch;
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

  first = (volatile Area15213PanelWork*)area15213WorkCursor_1F800044;
  if (first->unk_02 == 0) {
    u8 count;
    u8 next;

    count = first->unk_02;
    next = count + 1;
    FIELD_REF(u16, first, 0x3au) += D_80143E6C & 0xf;
    first->unk_02 = next;
  }
  if ((D_8014832E & 4) == 0) {
    return;
  }

  scratch = area15213WorkCursor_1F800044;
  layer = (s32)scratch->unk_0b - 2;
  scratch->unk_38 = scratch->unk_38 + 0x800 + (layer << 10);
  if (D_80104001 + 8 < FIELD_REF(s16, scratch, 0x3au)) {
    FIELD_REF(s16, scratch, 0x3au) = -8;
  }

  scratch = area15213WorkCursor_1F800044;
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
  x = (area15213WorkCursor_1F800044->unk_34 >> 9) - 0x3fc0;
  y = (area15213WorkCursor_1F800044->unk_38 >> 9) - 0x3fc0;

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
          FIELD_REF(s16, area15213WorkCursor_1F800044, 0x36u) - 2 + layer +
              column,
          FIELD_REF(s16, area15213WorkCursor_1F800044, 0x3au) - 2 + layer + row);
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

      *(u8*)(primitive + 0x0c) =
          D_801F4E30[layer] + column * D_801F4E38[layer];
      *(u8*)(primitive + 0x0d) =
          D_801F4E34[layer] + row * D_801F4E38[layer] -
          (D_801F4E38[layer] *
           FIELD_REF(u16, area15213WorkCursor_1F800044, 0x38u)) /
              0x10000;
      *(u8*)(primitive + 0x14) =
          D_801F4E38[layer] +
          (D_801F4E30[layer] + column * D_801F4E38[layer]);
      *(u8*)(primitive + 0x15) =
          D_801F4E34[layer] + row * D_801F4E38[layer] -
          (D_801F4E38[layer] *
           FIELD_REF(u16, area15213WorkCursor_1F800044, 0x38u)) /
              0x10000;
      *(u8*)(primitive + 0x1c) =
          D_801F4E30[layer] + column * D_801F4E38[layer];
      *(u8*)(primitive + 0x1d) =
          D_801F4E38[layer] +
          (D_801F4E34[layer] + row * D_801F4E38[layer] -
           (D_801F4E38[layer] *
            FIELD_REF(u16, area15213WorkCursor_1F800044, 0x38u)) /
               0x10000);
      *(u8*)(primitive + 0x24) =
          D_801F4E38[layer] +
          (D_801F4E30[layer] + column * D_801F4E38[layer]);
      *(u8*)(primitive + 0x25) =
          D_801F4E38[layer] +
          (D_801F4E34[layer] + row * D_801F4E38[layer] -
           (D_801F4E38[layer] *
            FIELD_REF(u16, area15213WorkCursor_1F800044, 0x38u)) /
               0x10000);

      func_80155A08(
          (FIELD_REF(s16, area15213WorkCursor_1F800044, 0x36u) - 2 + layer +
           column)
              << 16,
          (FIELD_REF(s16, area15213WorkCursor_1F800044, 0x3au) - 2 + layer +
           row)
              << 16,
          1, 0x28);
    }
  }
}
