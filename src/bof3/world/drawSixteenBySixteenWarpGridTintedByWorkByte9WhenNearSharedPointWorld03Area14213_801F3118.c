#include "bof3/bof3.h"
#include "gpu/prim.h"

#include <stdlib.h>

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * because the original re-materialises its address for every access with a
 * `lui %hi` plus `%lo` load pair instead of keeping one base live. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared overlay-focus halfwords at 0x8014930A/0x8014930E, the point the
 * sibling lifts compare against the work record halfwords 0x36/0x3A. The
 * original reads each once with a `lui`/`lh` pair before the frame is set up. */
extern s16 D_8014930A; /* @source 0x8014930A @kind unknown */
extern s16 D_8014930E; /* @source 0x8014930E @kind unknown */

/* Shared frame counter at 0x80143E6C, read once per projected axis. This body
 * takes its low byte after a left shift by four and scales it by the overlay
 * parity byte pair, which is the sine/cosine phase step it feeds the angular
 * helpers below. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Overlay byte pair at 0x801F355C: two signed phase scales selected by the
 * row parity of the grid walk and read with `lb` off a `%hi`/`%lo` base plus
 * the parity register. The pair sits at 0x801F355C inside the overlay's
 * trailing data span, whose table base 0x801F3554 Splat already publishes. */
extern s8 D_801F355C[]; /* @source 0x801F355C @kind data */

/* Scratchpad projection vector at 0x1F800014, the three halfwords RotTransPers
 * reads here. The original materialises its address as a value (lui plus
 * addiu %lo) and keeps it live across the grid walk, so it is taken by name. */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Shared grid-cell primitive linker: takes the cell origin pair, the kind and
 * the size class. This body links kind-1, 0xC-class cells. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive helpers: func_8017AA30 prepares the primitive just taken
 * from g_PrimCursor, and func_801782FC/func_801783C8 return the sine and
 * cosine of a 12-bit phase in the same fixed-point scale the >> 6 products
 * below reduce. */
void func_8017AA30(void* arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

s32 GetGraphType(void);

/* @source 0x801F3118
 * @behavior Overlay warp-grid drawer. It first measures the work record read
 *           through the pointer cell at 0x1F800044 against the shared point at
 *           0x8014930A/0x8014930E: the halfwords at record offsets 0x36 and
 *           0x3A are subtracted from those two shared halfwords, each
 *           difference is replaced by its magnitude, and the body returns as
 *           soon as their sum is not below 0x15. Otherwise it advances the
 *           record step byte at 0x09 by two, picks the draw-mode tpage argument
 *           by re-reading GetGraphType for graphics types 1 and 2 (0x2A5) and
 *           falling back to 0xB5, arms the shared cursor with
 *           SetDrawMode(g_PrimCursor, 0, 1, tpage, 0), and links a kind-1,
 *           0xC-class grid-cell primitive at the record cursor pair
 *           0x34/0x38. It then walks a sixteen by sixteen grid. The row loop
 *           re-reads the record once for the row tint, which is the complement
 *           of the step byte minus the row times sixteen, and takes the row
 *           parity as the selector into the overlay byte pair at 0x801F355C.
 *           Every cell takes the primitive cursor, seeds it with func_8017AA30,
 *           marks it semi-transparent and paints its colour bytes 0x04/0x05/
 *           0x06 with the row tint. The cell phase is the column shifted left
 *           by eight, offset by the low byte of the shared frame counter at
 *           0x80143E6C shifted left by four and multiplied by the selected
 *           signed parity byte; the sine of that phase, reduced by six and
 *           tripled, biased by -0x4000 and added to record word 0x34 shifted
 *           right by nine, becomes the scratch vector x, its cosine in the same
 *           form over record word 0x38 becomes y, and z is -0x180 minus twice
 *           the cell tint before complementing. RotTransPers projects the
 *           scratch vector into the primitive halfwords at 0x08 through two
 *           address-taken locals, the cell is linked again at 0xC, and the walk
 *           advances the column until sixteen, then the row until sixteen with
 *           the row bias grown by sixteen. The trailing epilogue re-runs the
 *           same tpage selection, arms SetDrawMode with dtd 0 and links the
 *           cursor once more. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenBySixteenWarpGridTintedByWorkByte9WhenNearSharedPointWorld03Area14213_801F3118(
    void) {
  u8* prim;
  SVECTOR* vtx;
  s32 graph_value;
  s32 row;
  s32 column;
  s32 tint;
  s32 parity;
  s32 row_bias;
  s32 phase;
  s32 projected;
  s32 depth;

  if (abs(D_8014930A - FIELD_REF(s16, D_1F800044, 0x36)) +
          abs(D_8014930E - FIELD_REF(s16, D_1F800044, 0x3a)) >= 0x15) {
    return;
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0x2a5;
  } else {
    graph_value = 0xb5;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, graph_value, 0);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                FIELD_REF(s32, D_1F800044, 0x38), 1, 0xc);

  D_1F800044[9] += 2;

  row = 0;
  vtx = (SVECTOR*)&D_1F800014;
  row_bias = 0;
  do {
    column = 0;
    tint = ~(D_1F800044[9] - row_bias) & 0xff;
    parity = row & 1;
    do {
      prim = g_PrimCursor;
      func_8017AA30(prim);
      SetSemiTrans(prim, 1);
      prim[4] = tint;
      prim[5] = tint;
      prim[6] = tint;

      phase = column << 8;
      vtx->vx = (FIELD_REF(s32, D_1F800044, 0x34) >> 9) - 0x4000 +
                (func_801782FC(phase + (((D_80143E6C << 4) & 0xff) *
                                        D_801F355C[parity])) >>
                 6) *
                    3;
      column++;
      vtx->vy = (FIELD_REF(s32, D_1F800044, 0x38) >> 9) - 0x4000 +
                (func_801783C8(phase + (((D_80143E6C << 4) & 0xff) *
                                        D_801F355C[parity])) >>
                 6) *
                    3;
      vtx->vz = -0x180 - (((D_1F800044[9] - row_bias) & 0xff) * 2);

      RotTransPers(vtx, (long*)(prim + 8), (long*)&projected,
                   (long*)&depth);

      func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                    FIELD_REF(s32, D_1F800044, 0x38), 1, 0xc);
    } while (column < 0x10);
    row++;
    row_bias += 0x10;
  } while (row < 0x10);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0x2a5;
  } else {
    graph_value = 0xb5;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, graph_value, 0);
  func_80155A08(FIELD_REF(s32, D_1F800044, 0x34),
                FIELD_REF(s32, D_1F800044, 0x38), 1, 0xc);
}
