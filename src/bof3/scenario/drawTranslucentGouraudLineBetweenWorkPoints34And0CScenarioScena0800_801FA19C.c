#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident helpers this overlay calls, declared the way its siblings declare
 * them: func_801AFE18 initialises the 0x20-byte projection scratch handed as
 * arg0, func_801AFF04 projects the three-word point handed as arg0 into the
 * screen halfword pair written at arg1, and func_8017AA94 stamps the LINE_G2
 * packet at the address it is handed before the primitive is written. */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_8017AA94(void* arg0);

/* @source 0x801FA19C
 * @behavior Emits one semi-transparent Gouraud line between the two work points
 * this overlay's movement handlers hand over. It first publishes the draw mode
 * GetTPage(0, 1, 0x3C0, 0x100) at the primitive cursor 0x8014598C through
 * SetDrawMode with both dither fields clear and a null texture window, queues
 * that 0x0C-byte draw-mode packet under OT slot 1 with func_8014E5A0, and
 * re-reads the cursor as the LINE_G2 packet the body then fills: func_8017AA94
 * stamps it, SetSemiTrans marks it translucent, and func_801AFE18 fills the
 * 0x20-byte projection scratch in the same 0x50-byte frame. Both arguments are
 * three-word positions projected in turn through func_801AFF04 into the same
 * eight-byte screen pair in the caller's second stack slot: the first argument
 * lands in x0/y0 at +0x08/+0x0A and the second in x1/y1 at +0x10/+0x12. The
 * first endpoint carries 0x40 in all three colour channels and the second
 * carries zero, so the line fades from that grey head to a black tail; the
 * packet closes with the 0x14-byte LINE_G2 size queued under OT slot 1. Takes
 * the two position pointers and returns nothing.
 *
 * Evidence: both call sites in this payload, subtractWorkWords34And0C...at
 * 0x801F9E50 and subtractWorkWord0C...at 0x801F9EC4, pass `addiu $a0, $a1,
 * 0x34` and `addiu $a1, $a1, 0xC` off the scratchpad work pointer loaded from
 * 0x1F800044, which fixes the two argument positions as that object's words and
 * makes both arguments address-taken pointers. The trailer sibling
 * advanceTrailProjectPointsFillHeadingsAndAverageAnglesScenarioScena0800_801F8D98
 * seeds its newest trail record from the same words 0x34/0x38/0x3C, so the first
 * argument is that trail head; the second word run at 0x0C is the moving point
 * the two handlers subtract from. The queued sizes are the 0x0C-byte draw-mode
 * packet and the 0x14-byte LINE_G2 size, which is the same (1, 0x0C) plus
 * (1, 0x14) pair the exact sibling emitTrailStrip writes around its Gouraud
 * lines, and the cursor is re-read after the append exactly as that sibling
 * re-reads it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudLineBetweenWorkPoints34And0CScenarioScena0800_801FA19C(
    const void* trailPoint, const void* movingPoint) {
  LINE_G2* line;
  u16      sxy[4];
  u8       scratch[0x20];

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, GetTPage(0, 1, 0x3C0, 0x100), 0);
  func_8014E5A0(1, 0xC);

  line = (LINE_G2*)g_PrimCursor;
  func_8017AA94(line);
  SetSemiTrans(line, 1);

  func_801AFE18(scratch);
  func_801AFF04(trailPoint, sxy);
  line->x0 = sxy[0];
  line->y0 = sxy[1];
  func_801AFF04(movingPoint, sxy);
  line->x1 = sxy[0];
  line->y1 = sxy[1];
  line->r0 = 0x40;
  line->g0 = 0x40;
  line->b0 = 0x40;
  line->r1 = 0;
  line->g1 = 0;
  line->b1 = 0;
  func_8014E5A0(1, 0x14);
}
