#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9688
 * @behavior Per-frame translucent Gouraud ring emitter that takes no arguments
 * and reads no state. It first publishes the draw mode GetTPage(0, 2, 0x3C0,
 * 0x100) at the primitive cursor through SetDrawMode with dfe 0, dtd 1 and a
 * null texture window, and appends that 0x0C-byte draw-mode packet through
 * func_8014E5A0 under allocation class 1. It then runs sixteen iterations of
 * one semi-transparent POLY_G4 emission: each iteration re-reads g_PrimCursor,
 * stamps the packet through SetPolyG4 and marks it translucent, and writes a
 * quad that spans the ring band between two samples of the turn. The first
 * vertex pair at +0x08/+0x0A and the second at +0x10/+0x12 take the inner and
 * outer corners the previous iteration left in the two local pairs, the phase
 * advances by 0x100 of the 0x1000-unit turn, and the third pair at +0x18/+0x1A
 * and the fourth at +0x20/+0x22 take the corners sampled at that new phase, so
 * neighbouring quads share their edges and the sixteen of them close one turn.
 * The inner corner is the screen centre 0xA0/0x78 offset by the sample scaled
 * by 0x50 in x and 0x3C in y, the outer corner by 0xF0 and 0xB4, each product
 * with the signed sampler result shifted right twelve, the fixed-point scale
 * those helpers carry. The first and third vertices carry black and the second
 * and fourth carry white, so the band fades from black at the inner rim to
 * white at the outer rim. Each quad is appended through func_8014E5A0 under
 * allocation class 1 with the 0x24-byte POLY_G4 size. The 0x40-byte frame
 * keeps ra and s0-s4 across the sampler calls plus the two 8-byte corner pairs,
 * and the 0x18-byte outgoing-argument area is what puts the first pair at
 * frame offset 0x18.
 *
 * Evidence: x takes func_801783C8 and y takes func_801782FC, the overlay-local
 * cosine and sine of a 12-bit phase in 4096 units, exactly as the exact
 * siblings drawTranslucentGouraudDiscOfSixteenTrianglesAroundProjectedRecordPoint
 * in this payload and drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenterFadingFromTintToBlack
 * write them; the four constants stored before the loop are the phase-0
 * corners that corroborate it, 0xA0 plus 0x50 and 0x78 plus nothing for the
 * inner pair, 0xA0 plus 0xF0 and 0x78 for the outer. The four corner locals are
 * unsigned, which is what makes the original re-read them with lhu, and the
 * black and white triplets are the literal 0x00 and 0xFF bytes. The sampler
 * results are single calls per corner, so each corner is one multiplication,
 * one twelve-bit shift and one centre add. The packet sizes are the 0x0C-byte
 * DR_MODE and the 0x24-byte POLY_G4, the same (1, size) append idiom the two
 * exact siblings use. The only reference to this span is the jal in
 * invokeHelperThenConditionallyAdvanceWorkByte1ScenarioScena0800_801F9618,
 * which passes no arguments, so the body stays argumentless and void.
 *
 * Boundary: the span runs 0x1E4 bytes and ends exactly at the next declared
 * row 0x801F986C, and it opens with the 0x40-byte frame setup and closes with
 * the ra restore and jr ra, so it is code rather than a data run; the data and
 * carve levers are declined on that evidence.
 *
 * Measured shapes: writing the colour fields as chained assignments measured
 * 113 of 121 instructions with the first difference inside the first triple,
 * because a chain emits its three stores in reverse field order (0x06, 0x05,
 * 0x04) while the original stores 0x04, 0x05, 0x06; the twelve separate
 * ascending assignments below reproduce the original schedule, as the sibling
 * drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenterFadingFromTintToBlack
 * recorded when it chose the chained form for its own reversed triples. The
 * two 8-byte corner pairs are what place the halfword slots at frame 0x18/0x1A
 * and 0x20/0x22 rather than contiguously.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudRingOfSixteenQuadsAroundScreenCenterFadingFromBlackToWhiteScenarioScena0800_801F9688(
    void) {
  POLY_G4* prim;
  u16      inner[4];
  u16      outer[4];
  u16      tpage;
  u16      angle;
  s32      phase;
  u8       i;

  tpage = GetTPage(0, 2, 0x3C0, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  func_8014E5A0(1, 0xC);
  inner[0] = 0xF0;
  inner[1] = 0x78;
  outer[0] = 0x190;
  outer[1] = 0x78;
  phase = 0;
  i = 0;
  do {
    prim = (POLY_G4*)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    prim->x0 = inner[0];
    prim->y0 = inner[1];
    prim->x1 = outer[0];
    prim->y1 = outer[1];
    phase += 0x100;
    angle = phase;
    inner[0] = 0xA0 + ((0x50 * func_801783C8(angle)) >> 12);
    inner[1] = 0x78 + ((0x3C * func_801782FC(angle)) >> 12);
    outer[0] = 0xA0 + ((0xF0 * func_801783C8(angle)) >> 12);
    outer[1] = 0x78 + ((0xB4 * func_801782FC(angle)) >> 12);
    prim->x2 = inner[0];
    prim->y2 = inner[1];
    prim->x3 = outer[0];
    prim->y3 = outer[1];
    prim->r0 = 0;
    prim->g0 = 0;
    prim->b0 = 0;
    prim->r1 = 0xFF;
    prim->g1 = 0xFF;
    prim->b1 = 0xFF;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    prim->r3 = 0xFF;
    prim->g3 = 0xFF;
    prim->b3 = 0xFF;
    func_8014E5A0(1, 0x24);
    i++;
  } while (i < 16);
}
