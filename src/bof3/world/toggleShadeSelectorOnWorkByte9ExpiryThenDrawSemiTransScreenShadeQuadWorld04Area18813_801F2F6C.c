#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 D_801F68A0;
extern u8 D_801F689C;
extern u8 D_801F4DF0[];

/* @source 0x801F2F6C
 * @behavior Overlay shade pass, entry 1 of the two-entry handler run at
 *           0x801F4DF4 that the dispatcher 0x801F2EEC indexes with work record
 *           byte 1. It post-increments work byte 9 and, on every call whose
 *           pre-increment value has reached 0x10, re-seeds the payload shade
 *           byte 0x801F68A0 with the byte at 0x801F4DF0 indexed by the selector
 *           0x801F689C, clears work byte 9 and clamps that selector from its
 *           post-increment value back to zero, so the selector walks the two
 *           table bytes 0x4F and 0x7F and the shade alternates across the
 *           sixteen calls between them. Entry 0 of the same run at 0x801F2F30
 *           is what seeds the pass: work byte 9 clear, work byte 1 set to 1,
 *           shade 0x0F and selector 0.
 *           Every call also opens the shared primitive cursor 0x8014598C as a
 *           draw-mode packet through SetDrawMode with dfe 0, dtd 1 and the
 *           texture page GetTPage(2, 2, 0x140, 0x140), reserves 0xC bytes at
 *           ordering index 6 through 0x8014E5A0, and stamps a 0x24-byte
 *           POLY_G4 on the cursor with SetPolyG4. The quad spans (0, 0) to
 *           (0x140, 0x140), fills the red byte of all four vertices with the
 *           payload shade 0x801F68A0 and clears green and blue, so each call
 *           re-queues a semi-transparent 320-by-320 red wash; SetSemiTrans on
 *           the cursor followed by 0x8014E5A0(2, 0x24) closes the pass. Takes
 *           no argument, returns nothing, and writes no payload state except
 *           work byte 9, the shade byte, the selector and the primitive cursor.
 *           Evidence from the original bytes at 0x801F2F6C: lui 0x1F800044 with
 *           lw of the pointer cell, lbu 0x9, addiu 1, sltiu 0x10 and bnez with
 *           the post-increment sb in its delay slot; the lbu of 0x801F689C
 *           feeding addu of %lo(D_801F4DF0) before the second lw of the pointer
 *           cell; sb $zero,0x9 of that reloaded cell; and lbu/addiu/sb of
 *           0x801F689C with beqz and a store of zero. The payload is a copy of
 *           0x801F3148 up to its closing 0x8014E5A0(2, 0x24): that sibling adds
 *           only a g_ScenarioProgress test with a call to 0x80196070 before the
 *           epilogue, and shares this body instruction for instruction.
 *           Matching note: the four vertices take their colour through two
 *           chained pairs, quad->r0 = quad->r1 = 0x801F68A0 and
 *           quad->r2 = quad->r3 = 0x801F68A0, which is why the payload loads
 *           0x801F68A0 twice and stores the later vertex of each pair first.
 *           Four separate shade reads measured 81 of 87 instructions plus 0x10
 *           bytes, two adjacent reads per pair 85 of 93 plus 0x18, and separate
 *           reads with the later vertex first 81 of 93 plus 0x18; the chained
 *           pairs measured 87 of 87 in 0x15C bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void toggleShadeSelectorOnWorkByte9ExpiryThenDrawSemiTransScreenShadeQuadWorld04Area18813_801F2F6C(
    void) {
  POLY_G4* quad;
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);
  if (slots[0x11][9]++ >= 0x10) {
    D_801F68A0 = D_801F4DF0[D_801F689C];
    slots[0x11][9] = 0;
    if (D_801F689C++ != 0) {
      D_801F689C = 0;
    }
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(2, 2, 0x140, 0x140), 0);
  GpuAppendPrim(6, 0xC);
  quad = (POLY_G4*)g_PrimCursor;
  SetPolyG4(quad);
  quad->r0 = quad->r1 = D_801F68A0;
  quad->g0 = quad->g1 = 0;
  quad->b0 = quad->b1 = 0;
  quad->r2 = quad->r3 = D_801F68A0;
  quad->g2 = quad->g3 = 0;
  quad->b2 = quad->b3 = 0;
  quad->x0 = 0;
  quad->y0 = 0;
  quad->x1 = 0x140;
  quad->y1 = 0;
  quad->x2 = 0;
  quad->y2 = 0x140;
  quad->x3 = 0x140;
  quad->y3 = 0x140;
  SetSemiTrans((POLY_G4*)g_PrimCursor, 1);
  GpuAppendPrim(2, 0x24);
}
