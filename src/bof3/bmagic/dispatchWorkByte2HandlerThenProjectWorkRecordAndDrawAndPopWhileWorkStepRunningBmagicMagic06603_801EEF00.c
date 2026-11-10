#include "bof3/bof3.h"

/* This overlay's work-record transform at 0x801EF784 (0xB0 bytes, still an asm
 * span of this target). It pushes the current matrix, builds the work record's
 * position from words 0x34 and 0x38 and its angle from halfword 0x3E, projects
 * that vector with RotTrans, keeps the record's own rotation through RotMatrix
 * and MulMatrix2 over the shared matrix at 0x801492E8, and installs the result
 * with SetRotMatrix and SetTransMatrix. It takes no arguments and returns
 * nothing; the PopMatrix that closes its PushMatrix is this function's. */
extern void func_801EF784(void); /* @source 0x801EF784 */

/* This overlay's work-record draw step at 0x801EF158 (0x62C bytes, still an asm
 * span of this target). It picks its draw-mode tpage from GetGraphType, seeds
 * SetDrawMode on the shared primitive cursor, publishes work word 0x14 and the
 * work byte 0xB index into the scratchpad cells the shared primitives read, and
 * emits the record's quads and triangles. It takes no arguments and returns
 * nothing. */
extern void func_801EF158(void); /* @source 0x801EF158 */

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map. The original materialises the cell with its own lui / lw pair before the
 * frame is created and never reads it again, so the plain named cell suffices. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Seven-entry code-pointer run at 0x801F006C, the work byte 2 stage handler
 * table this dispatcher indexes: entry 0 = 0x801EEF78, entry 1 = 0x801EEFF8,
 * entry 2 = 0x801EF04C, entry 3 = 0x801EF0A4, entry 4 = 0x801EF108, entry 5 =
 * 0x801EF878 and entry 6 = 0x801EF8C8. The run is the second word on of the
 * overlay's closing textbin row at 0x801F0064, whose first two words are the
 * work byte 1 table the dispatcher at 0x801EEEBC indexes; this function indexes
 * the word its scaled index is added to, so the declaration starts at 0x801F006C
 * and the index is not bounded. */
extern void (*D_801F006C[])(void); /* @source 0x801F006C @kind data */

/* Shared battle work-record pointer in main RAM, outside this overlay's image.
 * The remaining asm spans of this target still reference the address, so
 * Splat's generated undefined_syms_auto.txt binds it and no target-map row is
 * needed. Byte 0xB of that record is the work step the exact sibling at
 * 0x801EF0A4 rewrites (masks with 0x1F) and the draw step at 0x801EF158 indexes
 * with, and 0xFF is the step-finished value the bmagic family's exact sibling
 * dispatchWorkByte1HandlerThenUpdateAndDrawWhileWorkStepRunningBmagicMagic09403_801EEE4C
 * tests against the same cell. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EEF00
 * @behavior Entry 0 of this overlay's work byte 1 handler table at 0x801F0064,
 *           so it runs while the scratchpad work object's byte 1 holds 0. It
 *           reads the work-object pointer cell at 0x1F800044 before it creates
 *           its 0x18-byte frame, scales the work object's unsigned byte 2 by
 *           four and calls that entry of the seven-entry code-pointer run at
 *           0x801F006C with no arguments; the original does not bound the
 *           index. It then reads the shared battle work-record pointer at
 *           0x801EB4E0 and, while that record's byte 0xB differs from 0xFF (the
 *           step-finished value of this bmagic family), calls this overlay's
 *           transform step at 0x801EF784, its work-record draw step at
 *           0x801EF158 and PopMatrix, in that order and with no arguments.
 *           Both calls return nothing. It returns nothing through the jr $ra at
 *           0x801EEF70, reads or writes no other state, and keeps no locals
 *           beyond the saved $ra.
 *
 *           Shape and naming evidence: the work-byte dispatch through an
 *           overlay code-pointer run follows the exact bmagic sibling at
 *           0x801EEEBC of this target (same cell-then-scaled-byte indexing, same
 *           unbounded index) and the exact
 *           dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24
 *           (dispatch, then the overlay's transform and draw calls, then
 *           PopMatrix). The work byte 0xB != 0xFF gate, the shared
 *           D_801EB4E0 cell and the resulting "while the work step runs"
 *           naming follow the exact
 *           dispatchWorkByte1HandlerThenUpdateAndDrawWhileWorkStepRunningBmagicMagic09403_801EEE4C
 *           of the same bmagic family, which reads the same cell and tests the
 *           same terminal value. A byte search of every payload under
 *           out/binaries finds exactly one copy of this 0x78-byte span, this
 *           one, so the clone lever had no source. The span carries a prologue
 *           and a jr $ra, so the data lever does not apply, and its Splat row
 *           already ends where the function ends, so the carve lever does not
 *           either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenProjectWorkRecordAndDrawAndPopWhileWorkStepRunningBmagicMagic06603_801EEF00(
    void) {
  u8 *record;

  D_801F006C[g_battle_work[2]]();
  record = D_801EB4E0;
  if (record[0xB] != 0xFF) {
    func_801EF784();
    func_801EF158();
    PopMatrix();
  }
}
