#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name every other lifted body of this overlay uses. The
 * cell holds a pointer rather than the record itself, the original re-reads it
 * after the indirect call because the dispatched handler may republish it, and
 * both reads materialise the cell with their own lui 0x1F80 plus lw 0x44. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Tail of the code-pointer run at 0x801F03D8. The byte-1 dispatcher at
 * 0x801EFDB4 indexes the run's first three words (0x801EFDF8, 0x801EFE80 and
 * 0x801EFF88); this handler is that run's entry 0 and indexes the same run one
 * word further in, from 0x801F03DC, so its index 0 reaches 0x801EFE80, index 1
 * reaches 0x801EFF88 and index 2 reaches the zero word at 0x801F03E0. The words
 * from 0x801F03E4 onward are zero. Splat places the run as the single
 * `dispatchHandlerTable801F03BC` read-only block and already emits the
 * address-anchored label for this base word. */
extern void (*D_801F03DC[])(void); /* @source 0x801F03DC @kind data */

/* Projects the work record's position and installs the resulting transform:
 * pushes a matrix, reads the scratchpad work record through the pointer cell at
 * 0x1F800044 and builds the rotation from its byte 9 and its words 0x34, 0x38
 * and halfword 0x3E through RotTrans, RotMatrix and MulMatrix2, then arms
 * SetRotMatrix and SetTransMatrix. */
extern void func_801F00A8(void);

/* Draws the work record's geometry under the transform installed by
 * func_801F00A8, choosing the draw mode from GetGraphType. */
extern void func_801F0164(void);

/* @source 0x801EFDF8
 * @behavior Dispatches one step of the overlay work handler and then draws the
 *   active record. The work-object pointer cell at 0x1F800044 is read, the
 *   record's unsigned byte at offset 0x2 is scaled by four and selects one entry
 *   of the code-pointer run word at 0x801F03DC, which is called with no
 *   arguments; the original does not bound the index. The cell is then read
 *   again, because the dispatched handler may republish it, and the record's
 *   bytes at offsets 0x0 and 0x9 are tested on that second pointer, so only when
 *   both are non-zero does the draw arm run: it calls the projection at
 *   0x801F00A8, the draw step at 0x801F0164 and PopMatrix, all with no
 *   arguments. The 0x18-byte frame only keeps $ra at 0x10 across the indirect
 *   call and the first load of the cell precedes frame creation; the 136-byte
 *   span stores nothing itself and the delay slots after both condition bytes
 *   stay empty.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawAndPopWhenWorkBytes0And9SetBmagicMagic01303_801EFDF8(
    void) {
  u8 *step_record;
  u8 *draw_record;

  step_record = g_battle_work;
  D_801F03DC[step_record[2]]();
  draw_record = g_battle_work;
  if (draw_record[0] != 0 && draw_record[9] != 0) {
    func_801F00A8();
    func_801F0164();
    PopMatrix();
  }
}
