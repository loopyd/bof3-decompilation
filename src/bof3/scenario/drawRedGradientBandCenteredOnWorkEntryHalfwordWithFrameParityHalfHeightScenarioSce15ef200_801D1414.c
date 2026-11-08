#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared per-frame counter; only its low bit is read here. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

void func_8014E5A0(u8 arg0, u8 arg1);

/* @source 0x801D1414
 * @behavior Draws the red gradient band of one work-table entry. It takes the
 * 0x24-byte POLY_G4 published at the primitive cursor cell 0x8014598C, stamps
 * it with SetPolyG4, clears its semi-transparency bit through SetSemiTrans with
 * zero and fills the first quad as the full-width 0x140 slab that ends on the
 * entry halfword at +0x4: both top vertices sit at that halfword minus the
 * entry byte at +0x3 minus the low bit of the shared per-frame counter at
 * 0x80143E6C, both bottom vertices sit on the halfword, the top pair stays
 * black and the bottom pair carries red 0x80 with green and blue cleared, and
 * func_8014E5A0 then appends the quad under OT slot 7. It reloads the cursor
 * cell and repeats with the mirrored second quad, whose top pair carries red
 * 0x80 and whose bottom pair sits at the halfword plus that same entry byte
 * plus the counter bit over black, so the pair is one vertically symmetric
 * red-to-black band centred on the entry halfword with a half height of the
 * entry byte plus the frame parity. The halfword and the byte are reloaded for
 * every vertex because the stores through the loaded cursor may alias the
 * entry, which the caller passes in $a0 from the jal at 0x801D1754. It returns
 * nothing and its 0x38-byte frame only keeps $ra and $s0-$s4 of the two quads.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the first measured shape was the same statements with all four
 * x stores written ahead of the y stores, and that order measured 82 of 104
 * instructions at 416 bytes; writing the coordinates in vertex order - x0/y0,
 * x1/y1, x2/y2, x3/y3 - measured 103 of 103 at the original 412 bytes. The
 * 0x38-byte frame keeps 0x20 bytes of local area, gcc reserves 0x10 for this
 * body, and nothing below reads or writes the missing 16 bytes, so they come
 * from a declaration the original translation unit carried that emitted no
 * code. Only that size is constrained: the name and type of `frame_reserve`
 * are a reconstruction choice flagged for review, and removing it leaves the
 * prologue and epilogue frame offsets as the only residual.
 */
void drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414(
    u8* record) {
  POLY_G4* prim;
  u8 jitter;
  u8 frame_reserve[0x10]; /* size-only frame reservation, see the note above */

  jitter = D_80143E6C & 1;
  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 0);
  prim->x0 = 0;
  prim->y0 = FIELD_REF(u16, record, 0x4) - FIELD_REF(u8, record, 0x3) - jitter;
  prim->x1 = 0x140;
  prim->y1 = FIELD_REF(u16, record, 0x4) - FIELD_REF(u8, record, 0x3) - jitter;
  prim->x2 = 0;
  prim->y2 = FIELD_REF(u16, record, 0x4);
  prim->x3 = 0x140;
  prim->y3 = FIELD_REF(u16, record, 0x4);
  prim->r0 = 0;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = 0;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = 0x80;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0x80;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(7, 0x24);

  prim = (POLY_G4*)g_PrimCursor;
  SetPolyG4(prim);
  SetSemiTrans(prim, 0);
  prim->x0 = 0;
  prim->y0 = FIELD_REF(u16, record, 0x4);
  prim->x1 = 0x140;
  prim->y1 = FIELD_REF(u16, record, 0x4);
  prim->x2 = 0;
  prim->y2 =
      FIELD_REF(u16, record, 0x4) + (FIELD_REF(u8, record, 0x3) + jitter);
  prim->x3 = 0x140;
  prim->y3 =
      FIELD_REF(u16, record, 0x4) + (FIELD_REF(u8, record, 0x3) + jitter);
  prim->r0 = 0x80;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r1 = 0x80;
  prim->g1 = 0;
  prim->b1 = 0;
  prim->r2 = 0;
  prim->g2 = 0;
  prim->b2 = 0;
  prim->r3 = 0;
  prim->g3 = 0;
  prim->b3 = 0;
  func_8014E5A0(7, 0x24);
}
