#include "bof3/bof3.h"
#include "gpu/prim.h"

void func_8017A9A4(void* arg0);
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F38A0
 * @behavior Draws one work record of the 0xE-stride record table at 0x800E4800
 *           as a flat two-pixel-wide vertical quadrangle at the current
 *           primitive cursor: takes the record's +0x0 halfword as the left edge
 *           and that edge + 2 as the right edge of both point pairs, spans the
 *           vertical pair from the +0x2 halfword minus the +0x4 halfword up to
 *           the +0x2 halfword plus the +0x4 halfword, and clamps that span to
 *           the record's own +0x8 and +0xA halfwords (each clamp re-publishes
 *           the bound into both of its point y values), stores 0x80 into the
 *           red, green and blue bytes, clears the semi-transparency bit and
 *           links the 0x18-byte packet as grid kind 4 through the shared
 *           grid-cell linker func_80155A08 with the target-local word pair at
 *           0x801F53A0/0x801F53A4 as its two coordinates. The overlay's ticker
 *           func_801F39D8 (jal at 0x801F3A10) calls it once per 0xE-stride
 *           record whose +0xC countdown halfword is still running, passing the
 *           record base in $a0, and then steps that record's +0x2 halfword down
 *           by +0x6 while growing +0x4, so the drawn bar keeps a fixed bottom
 *           bound and grows its top as the countdown runs out.
 * @status partial
 * @match 97.06
 * @residual The first 44 instructions match. The linker kind constant
 *           (`li a2,4`) follows the first coordinate load instead of filling
 *           the left-edge load delay slot at +0xB0. That slot contains `nop`,
 *           leaving 66 matching instructions and 272 bytes against 268.
 */
void drawClampedWorkRecordVerticalBarWorld01Area06714_801F38A0(u8* rec) {
  POLY_F4* prim;
  s16 top;
  s16 bottom;
  u16 left;
  u16 right;

  prim = (POLY_F4*)g_PrimCursor;
  func_8017A9A4(prim);
  SetSemiTrans(prim, 0);
  top = FIELD_REF(u16, rec, 0x2u) - FIELD_REF(u16, rec, 0x4u);
  prim->y1 = top;
  prim->y0 = top;
  if (top < FIELD_REF(s16, rec, 0x8u)) {
    top = FIELD_REF(s16, rec, 0x8u);
    prim->y1 = top;
    prim->y0 = top;
  }
  bottom = FIELD_REF(u16, rec, 0x2u) + FIELD_REF(u16, rec, 0x4u);
  prim->y3 = bottom;
  prim->y2 = bottom;
  if (bottom > FIELD_REF(s16, rec, 0xAu)) {
    bottom = FIELD_REF(s16, rec, 0xAu);
    prim->y3 = bottom;
    prim->y2 = bottom;
  }
  do {
    left = FIELD_REF(u16, rec, 0x0u);
    prim->x2 = left;
    prim->x0 = left;
    right = FIELD_REF(u16, rec, 0x0u) + 2;
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    prim->x3 = right;
    prim->x1 = right;
  } while (0);
  func_80155A08(PSX_REF(s32, 0x801F53A0u), PSX_REF(s32, 0x801F53A4u), 4, 0x18);
}
