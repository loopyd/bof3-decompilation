#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materializes this cell
 * with its own lui/lw pair before the dispatch index and again after each of
 * the two calls that follow, so it is read through this named cell instead of
 * the constant-address accessor, whose folded address stays live across a call
 * in a saved register and grows the frame. Other bmagic targets name the same
 * cell through the same row. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Four-entry work-byte-2 handler table for this overlay, based at 0x801EF424:
 * entry 0 = 0x801EEEC8, entry 1 = 0x801EEF58, entry 2 = 0x801EEFC4, entry 3 =
 * 0x801EF014. The original adds the scaled byte to this word, so the extern is
 * declared at the indexed entry, not at 0x801EF420, the start of the five-word
 * code-pointer run whose first word is this function itself. */
extern void (*D_801EF424[])(void); /* @source 0x801EF424 @kind data */

/* Shared main-executable helper the overlay calls once the work object is
 * active. Unlifted and unnamed, so it keeps its raw name and this target's map
 * row; this caller passes no arguments and reads no result. */
void func_800AF764(void);

/* This overlay's draw routine at 0x801EF08C: reads the work object's motion
 * pair through the same pointer cell and emits the sprite primitives. Unlifted,
 * so it keeps its raw name; it takes no arguments here. */
void func_801EF08C(void);

/* @source 0x801EEE28
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044 and
 *   uses the work object's unsigned byte at offset 0x2, scaled by four, to
 *   select one entry of the four-entry handler table based at 0x801EF424
 *   (0x801EEEC8, 0x801EEF58, 0x801EEFC4, 0x801EF014), which it calls with no
 *   arguments; the original does not bound the index. When that handler returns
 *   it reads the pointer cell a second time and, only when the work object's
 *   byte at offset 0x0 and its byte at offset 0x2 are both non-zero, calls the
 *   shared main-executable helper at 0x800AF764, reads the pointer cell a third
 *   time, adds 8 to the unsigned halfword at offset 0x2E and subtracts 0x10
 *   from the unsigned halfword at offset 0x30, and calls this overlay's draw
 *   routine at 0x801EF08C with the 0x30 store filling that call's delay slot.
 *   The three pointer reads are separate locals because the original
 *   materializes the cell once per reload point, and the calls in between are
 *   what invalidate the earlier values. The motion pair at 0x2E and 0x30 is the
 *   8.8 sprite offset that 0x801EF08C scales and adds to the primitive
 *   position. The pointer load precedes frame creation; the 0x18-byte frame
 *   only keeps $ra at 0x10 across the three calls. Nothing is returned and no
 *   other state is read or written.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenShiftMotionPairAndDrawWhenActiveBmagicMagic15903_801EEE28(
    void) {
  u8 *stepWork;
  u8 *gateWork;
  u8 *motionWork;
  u16 x;
  u16 y;

  stepWork = g_battle_work;
  D_801EF424[stepWork[2]]();
  gateWork = g_battle_work;
  if (gateWork[0] != 0 && gateWork[2] != 0) {
    func_800AF764();
    motionWork = g_battle_work;
    x = *(u16 *)(motionWork + 0x2E);
    y = *(u16 *)(motionWork + 0x30);
    *(u16 *)(motionWork + 0x2E) = x + 8;
    *(u16 *)(motionWork + 0x30) = y - 0x10;
    func_801EF08C();
  }
}
