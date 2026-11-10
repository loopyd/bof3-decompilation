#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, spelled through this address-derived
 * symbol, the same named cell the exact lifts of this target already use. A named
 * cell symbol lets the assembler fold the constant address into each load
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)), and the original re-reads the cell after
 * the byte store inside the guarded arm, because a store through the pointer it
 * supplied may alias the cell object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Full-screen fade-tile drawer of this overlay, still an asm boundary in this
 * target, so it keeps its raw name. Its body selects the semi-transparent draw
 * mode, emits a 320x240 tile at the primitive cursor and scales work-object byte
 * 0x9 by 15 into the tile RGB bytes. */
void func_801EFD88(void);

/* @source 0x801EF234
 * @behavior Third entry, index 2, of the six-pointer work-step run at
 *           0x801EEC04 (0x801EECC0, 0x801EF0F8, 0x801EF234, 0x801EF2A8,
 *           0x801EF31C, 0x801EF350), which
 *           dispatchWorkByte1HandlerBmagicMagic02003_801EEC48 copies into its
 *           own 0x30-byte frame and indexes with work-object unsigned byte 1, so
 *           this body runs with no arguments and returns no value. It first calls
 *           the fade-tile drawer at 0x801EFD88 with no arguments. It then loads
 *           the work-object pointer cell at 0x1F800044 once and tests work byte
 *           0x0B for zero with an equality test on the loaded byte; while that
 *           byte is non-zero it returns through the jr $ra at 0x801EF2A0 having
 *           written nothing. On the zero arm it decrements work byte 9 (lbu 9 /
 *           addiu -1 / sb 9) and tests the truncated byte it just stored, an andi
 *           0xFF of the stored register with no second load of the cell, so the
 *           step is skipped on every frame but the last. Only on that frame does
 *           it re-read the 0x1F800044 cell and increment work byte 1 at byte
 *           width, which advances the outer dispatcher to step 3 at 0x801EF2A8.
 *           The 0x18-byte frame keeps only $ra at 0x10, and every branch and call
 *           delay slot in the span is nop. The roles of work byte 0x0B and work
 *           byte 9 are not established, so they stay offset-anchored. A byte
 *           search over every payload under out/binaries found exactly one copy
 *           of this 0x74-byte span, so the clone lever was declined. The span
 *           opens with addiu $sp,-0x18 and closes in jr $ra through its own
 *           frame, so the data lever did not apply, and it sits between the
 *           reviewed boundaries at 0x801EF0F8, whose span ends at this address,
 *           and 0x801EF2A8, so the carve lever did not apply either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFadeTileThenDecrementWorkByte9AndAdvanceWorkByte1WhenWorkByteBIsZeroBmagicMagic02003_801EF234(
    void) {
  u8 *work;
  u8 countdown;
  u8 *object;

  func_801EFD88();
  work = D_1F800044;
  if (work[0xB] == 0) {
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
      object = D_1F800044;
      object[1] += 1;
    }
  }
}
