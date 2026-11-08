#include "bof3/bof3.h"

/* The overlay handler run at 0x801D1B1C is one fourteen-word code-pointer run,
 * so this dispatcher base 0x801D1B38 is the run word 7 that armCountdown10ClearScreenPairAndAdvanceWorkByte
 * (0x801D0F68, entry 7) occupies. The target's Splat `textbin` row labels the
 * run's first eight words func_801D1B1C and the remaining six sit in the
 * neighbour span labelled func_801D1B3C, so no reviewed data row places this
 * dispatcher's base; the generated D_ row for its interior word and this
 * reference are all that bind it. */
extern void (*D_801D1B38[])(void); /* @source 0x801D1B38 @kind data */

/* The draw helper at 0x801D17A0 is an exact lift: its body takes the two signed
 * halfword arc extents this call site loads with `lh` and the u8 shade byte this
 * call site's literal 0x80 feeds. */
void drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0(
    s16 half_width, s16 half_height, u8 shade);

/* @source 0x801D0EF8
 * @behavior Overlay step dispatch on work-record byte 1 of this payload, then
 * the reported screen pair: it loads the work-object pointer published at the
 * scratchpad pointer cell 0x1F800044 before it creates its frame, takes that
 * object's unsigned byte at offset +0x1 (the same byte the overlay entry
 * func_801D0C04 dispatches on and that the step handlers advance), scales it by
 * four and calls that entry of the handler run based at 0x801D1B38 with no
 * arguments, discarding any result. That base is word 7 of the same run whose
 * word 4 the exact sibling dispatchWorkByte1HandlerScenarioSce15ef200_801D0DA4
 * indexes, so the reachable entries are the arm and draw step handlers
 * 0x801D0F68, 0x801D0F9C, 0x801D1008, 0x801D1050, 0x801D10E8, 0x801D1118 and
 * 0x801D11A4 (entries 7 through 13); the original does not bound the index
 * before the scaled table load. It then reloads the same pointer cell into a
 * second local, because the indirect call may republish that cell, and gates on
 * the work byte at +0x1 again: while that byte is non-zero it hands the draw
 * helper drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0
 * the signed halfwords at +0x2E and +0x30 (the reported x and y that sibling
 * lifts of this work object read there) and the literal 0x80. It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra at 0x10
 * across the two calls.
 *
 * Evidence: the table base immediate 0x801D1B38 is the fourth word of the run
 * from word 4, and the payload's only in-image pointer run is the fourteen
 * words at 0x801D1B1C through 0x801D1B50. No `jal` and no word of this payload
 * addresses 0x801D0EF8, so the overlay loader outside the payload enters it,
 * exactly as it enters the two sibling step dispatchers. The draw helper stores
 * its third argument into an SPRT colour byte and its first two into scaled
 * primitive positions, which is what fixes the argument widths here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerThenDrawScreenPairScenarioSce15ef200_801D0EF8(void) {
  u32 slot_offset;
  u8* work;
  u8* next;

  slot_offset = 0x44u;
  work = PSX_REF(u8*, SPAD_BASE + slot_offset);
  D_801D1B38[work[1]]();
  next = PSX_REF(u8*, SPAD_BASE + slot_offset);
  if (next[1] != 0) {
    drawTranslucentFanOfThirtyTwoGouraudTrianglesAroundScreenBottomCenterScenarioSce15ef200_801D17A0(
        *(s16*)(next + 0x2E), *(s16*)(next + 0x30), 0x80);
  }
}
