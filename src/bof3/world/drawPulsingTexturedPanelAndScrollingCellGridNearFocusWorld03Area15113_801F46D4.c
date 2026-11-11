#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

typedef struct Area15113PanelWork {
  u8 unk_00[2];
  u8 unk_02;
  u8 unk_03[8];
  u8 unk_0b;
  u8 unk_0c[0x28];
  s32 unk_34;
  s32 unk_38;
} Area15113PanelWork;

/* @source 0x1F800044 @kind data */
extern Area15113PanelWork* D_1F800044;
/* @source 0x1F800014 @kind data */
extern SVECTOR D_1F800014[];
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
/* @source 0x801F5504 @kind data */
extern u8 D_801F5504[];
/* @source 0x801F5508 @kind data */
extern u8 D_801F5508[];
/* @source 0x801F550C @kind data */
extern u8 D_801F550C[];
/* @source 0x80155560 */
extern void func_80155560(u32 command, void* primitive, s32 mode);
/* @source 0x801559AC */
extern void* func_801559AC(s32 column, s32 row);
/* @source 0x80155A08 */
extern void func_80155A08(s32 column, s32 row, s32 mode, s32 size);

/* @source 0x801F46D4
 * @behavior Initializes work byte 2 and offsets the work halfword at 0x3A by
 * the frame counter's low nibble. When shared byte 0x8014832E bit 4 is set,
 * advances the work word at 0x38 by 0x800 plus (work byte 0x0B minus 2)
 * shifted left ten and wraps the halfword at 0x3A to -8 above the map height
 * at 0x80104001 plus 8. Near the shared focus halfwords 0x8014930A and
 * 0x8014930E it projects one pulsing textured quad at depth -0x300 through
 * RotTransPers4 over the scratchpad vectors at 0x1F800014, whose halfsize
 * pulses by |0x0F - (frame counter & 0x1F)| around the projected work words
 * at 0x34 and 0x38 scaled by >> 9 and biased by -0x3FC0; the quad is stamped
 * with clut 0x78CB, tpage 0x12B for graphics types 1 and 2 and 0x5B
 * otherwise, flat colour 0x28, and is handed to 0x80155560 with the layer
 * byte out of 0xBB509100 before the primitive is reserved through
 * GpuAppendPrim(4, 0x28). It then walks a (2 - layer) * 4 + 1 by
 * (2 - layer) * 4 grid: for each cell it looks the two work halfwords at
 * 0x36 and 0x3A biased by -2 + layer plus the cell column and row up in the
 * shared 0x801559AC lookup, and, when that returns a record, re-seeds the
 * current primitive from g_PrimCursor, copies the four projected position
 * words at offsets 0x08/0x10/0x18/0x20 of the record into it, fills its
 * texture window from the target-local byte columns D_801F5504 (u origin),
 * D_801F5508 (v origin) and D_801F550C (tile size) at index layer, scrolling
 * v by (size * (work halfword at 0x38)) / 0x10000, and queues the cell
 * through 0x80155A08 with both coordinates shifted left by 16. Takes no
 * arguments and returns nothing.
 * @status partial
 * @match 83.03
 * @residual Expected 323/389 instructions at 1556 bytes, first difference at
 * +0x08C on the work byte 0x0B load, as measured for this same body in
 * emi/world00/area016/13@0x801F4768 and emi/world00/area033/13@0x801F469C; the
 * donor evidence is that this span differed from the area033 span in exactly
 * sixteen lbu immediates, the three target-local table bases. Smallest
 * missing evidence: the original allocation of layer across the scroll and
 * clamp block, where the original loads the byte into a scratch register and
 * subtracts into a callee-saved one.
 */
void drawPulsingTexturedPanelAndScrollingCellGridNearFocusWorld03Area15113_801F46D4(
    void) {
  volatile Area15113PanelWork* first_frame;
  Area15113PanelWork* scratch;
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

  first_frame = D_1F800044;
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

  scratch = D_1F800044;
  layer = (s32)scratch->unk_0b - 2;
  scratch->unk_38 = scratch->unk_38 + 0x800 + (layer << 10);
  if (D_80104001 + 8 < FIELD_REF(s16, scratch, 0x3au)) {
    FIELD_REF(s16, scratch, 0x3au) = -8;
  }

  scratch = D_1F800044;
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
  x = (D_1F800044->unk_34 >> 9) - 0x3fc0;
  y = (D_1F800044->unk_38 >> 9) - 0x3fc0;

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
          FIELD_REF(s16, D_1F800044, 0x36u) - 2 + layer + column,
          FIELD_REF(s16, D_1F800044, 0x3au) - 2 + layer + row);
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

      *(u8*)(primitive + 0x0c) = D_801F5504[layer] + column * D_801F550C[layer];
      *(u8*)(primitive + 0x0d) = D_801F5508[layer] +
                                 row * D_801F550C[layer] -
                                 (D_801F550C[layer] *
                                  FIELD_REF(u16, D_1F800044, 0x38u)) /
                                     0x10000;
      *(u8*)(primitive + 0x14) =
          D_801F550C[layer] +
          (D_801F5504[layer] + column * D_801F550C[layer]);
      *(u8*)(primitive + 0x15) = D_801F5508[layer] +
                                 row * D_801F550C[layer] -
                                 (D_801F550C[layer] *
                                  FIELD_REF(u16, D_1F800044, 0x38u)) /
                                     0x10000;
      *(u8*)(primitive + 0x1c) = D_801F5504[layer] + column * D_801F550C[layer];
      *(u8*)(primitive + 0x1d) =
          D_801F550C[layer] +
          (D_801F5508[layer] + row * D_801F550C[layer] -
           (D_801F550C[layer] * FIELD_REF(u16, D_1F800044, 0x38u)) / 0x10000);
      *(u8*)(primitive + 0x24) =
          D_801F550C[layer] +
          (D_801F5504[layer] + column * D_801F550C[layer]);
      *(u8*)(primitive + 0x25) =
          D_801F550C[layer] +
          (D_801F5508[layer] + row * D_801F550C[layer] -
           (D_801F550C[layer] * FIELD_REF(u16, D_1F800044, 0x38u)) / 0x10000);

      func_80155A08(
          (FIELD_REF(s16, D_1F800044, 0x36u) - 2 + layer + column) << 16,
          (FIELD_REF(s16, D_1F800044, 0x3au) - 2 + layer + row) << 16, 1, 0x28);
    }
  }
}
