#include "bof3/bof3.h"
#include "gpu/prim.h"

extern int rand(void);

/* Scratchpad work-record pointer cell at 0x1F800044. The original
 * re-materializes the cell with its own lui/lw pair at every use, so the
 * constant-address accessor's folded address would stay live across a call in
 * a saved register and grow the frame; the cell is read through this named row
 * instead, as in the exact bmagic sibling
 * dispatchWorkByte2HandlerThenDrawWorkRecordAndPopWhenWorkBytes2And0SetBmagicMagic09703_801EEFC8. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword at 0x1F800000, the signed radius this body scales every
 * sampled offset by. The original seeds the cell with the absolute-constant
 * store form (lui $at,0x1F80 plus the offset) but reaches it inside the loop
 * through a register base it builds once as a %hi/%lo pair four bytes of the
 * body apart, so the cell is bound by name for the loop reads. The generated
 * undefined_syms_auto.txt row for 0x1F800000 already covers it: three asm rows
 * of this target (0x801EF5C4, 0x801EFE04 and 0x801F0568) build the same base. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared engine primitive stamper, called on the fresh cursor primitive before
 * this body writes its fields. The payload names 0x8017A97C only from spans
 * that are still asm rows of this target, so it keeps its raw name and the
 * generated undefined_funcs_auto.txt row for that address. */
void func_8017A97C(void *arg0);

/* Shared grid-cell primitive linker: it takes the record cell origin pair, the
 * kind and the packet size, links the primitive at g_PrimCursor and advances
 * the cursor by that size, which is why this body re-reads g_PrimCursor at the
 * top of every iteration. It links a 0xC-byte draw-mode packet once and then
 * one 0x1C-byte POLY_G3 packet per triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the >> 12 products below. This body takes the
 * 0x801782FC sample for both vertex x sums and the 0x801783C8 sample for both
 * vertex y sums. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Naming and shape evidence: the payload holds no byte-identical twin of this
 * 644-byte body - a whole-span comparison against the other 395 payloads under
 * out/binaries/ finds no occurrence - so the clone lever has no source to copy
 * and the body was seeded from the original bytes. The nearest exact siblings
 * are the drawing steps of the same overlay family:
 * drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic07003_801EF810
 * for the published centre, the draw-mode disjunction over GetGraphType and the
 * 0xC-byte then 0x1C-byte linker calls, and
 * drawEightRotatedTrianglesFromPublishedCenterTintedByPaletteRowAtWorkBytes3And4BmagicMagic16203_801EFCD8
 * for the eight-iteration POLY_G3 fan, the sample phase shifted left 9 and the
 * cursor re-read at the top of every iteration. The name keeps the sibling
 * spelling convention under this target and address: a translucent Gouraud fan
 * of eight triangles around the work record screen pair, its centre tinted from
 * work byte 0x0A and its radius jittered by the low three bits of rand. */
/* @source 0x801F083C
 * @behavior Battle-magic drawing step reached from the draw branch of the
 *           overlay dispatcher at 0x801EEFC8, which shifts the work record
 *           halfword 0x2E by 0x10, calls this body, shifts it back by 0x20 and
 *           calls this body again, so the same fan is drawn at two vertical
 *           offsets; it takes no arguments and returns nothing. It first seeds
 *           the scratchpad halfword at 0x1F800000 with the low three bits of
 *           rand plus four times work byte 0x0A, which is the radius every
 *           sampled offset is scaled by. It then picks the draw-mode tpage
 *           argument 0xA5 when GetGraphType returns 1 and 0xA5 again when it
 *           returns 2, and 0x35 otherwise, seeds SetDrawMode at g_PrimCursor
 *           with dfe 0 and dtd 1, then clears the triangle counter, takes the
 *           address of the scratchpad halfword as the signed radius base, and
 *           links the 0xC-byte draw-mode packet at the record words 0x34 and
 *           0x38 with kind 2. Then it runs eight iterations of one POLY_G3
 *           triangle each: every iteration takes a fresh g_PrimCursor
 *           primitive, stamps it through func_8017A97C and marks it
 *           semi-transparent, puts the record halfwords 0x2E and 0x30 into the
 *           primitive halfwords 0x08 and 0x0A as the shared centre vertex, adds
 *           the func_801782FC and func_801783C8 samples of the counter shifted
 *           left 9, scaled by the signed radius cell and shifted right 12, into
 *           the primitive halfwords 0x10 and 0x12, increments the counter, adds
 *           the same two samples of the incremented counter into the primitive
 *           halfwords 0x18 and 0x1A, so the triangles of neighbouring
 *           iterations meet at the shared sample and the eight of them span the
 *           whole turn, writes twelve times work byte 0x0A into colour bytes
 *           0x04 and 0x05 and ten times work byte 0x0A into byte 0x06 for the
 *           tinted centre vertex, writes one into bytes 0x0C, 0x0D, 0x0E, 0x14,
 *           0x15 and 0x16 for the near-black rim vertices, and links the
 *           0x1C-byte POLY_G3 packet at the record words 0x34 and 0x38 with
 *           kind 2 until the counter reaches eight. It keeps a 0x30-byte frame
 *           for ra, the four saved registers 0x18 to 0x28 and the fifth
 *           draw-mode argument at 0x10, and writes no state outside the radius
 *           cell, the cursor and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundWorkScreenPairTintedByWorkByteAWithJitteredRadiusBmagicMagic09703_801F083C(
    void) {
  POLY_G3 *primitive;
  s16 *radius;
  s32 phase;
  s32 tpage;
  s32 i;

  *(u16 *)0x1F800000 = (rand() & 7) + g_battle_work[0xA] * 4;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  i = 0;
  radius = (s16 *)&D_1F800000;
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                FIELD_REF(s32, g_battle_work, 0x38u), 2, 0xC);

  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);
    primitive->x0 = FIELD_REF(u16, g_battle_work, 0x2Eu);
    primitive->y0 = FIELD_REF(u16, g_battle_work, 0x30u);
    phase = i << 9;
    primitive->x1 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y1 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);
    i++;
    phase = i << 9;
    primitive->x2 =
        FIELD_REF(u16, g_battle_work, 0x2Eu) +
        ((func_801782FC(phase) * *radius) >> 12);
    primitive->y2 =
        FIELD_REF(u16, g_battle_work, 0x30u) +
        ((func_801783C8(phase) * *radius) >> 12);

    primitive->r0 = g_battle_work[0xA] * 12;
    primitive->g0 = g_battle_work[0xA] * 12;
    primitive->b0 = g_battle_work[0xA] * 10;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = 1;
    primitive->g2 = 1;
    primitive->b2 = 1;

    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34u),
                  FIELD_REF(s32, g_battle_work, 0x38u), 2, 0x1C);
  } while (i < 8);
}
