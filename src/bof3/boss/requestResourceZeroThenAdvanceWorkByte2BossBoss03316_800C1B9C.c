#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so the load folds to its own lui plus an offset load, which is what
 * the original does after the resource request. Splat binds the address through
 * this target's generated undefined_syms_auto.txt, so no target-map row is added
 * here. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-executable resource request helper, reached through the generated
 * undefined_funcs_auto row this target already carries because its remaining asm
 * spans call 0x8014D6B8 too. The sibling boss and bmagic lifts declare the same
 * prototype and pass a resource id. */
extern void func_8014D6B8(u32 flag);

/* @source 0x800C1B9C
 * @behavior Byte-2 handler entry 0 of this overlay's code-pointer run based at
 *           0x800C2798, the base word the byte-2 dispatcher
 *           dispatchStateByte2Table2798BossBoss03316_800C1B58 loads and indexes
 *           with work byte 2 scaled by four; run word 0 at 0x800C2798 holds this
 *           address, and no jal in this payload names it. It requests resource 0
 *           through the shared main-executable helper func_8014D6B8 and drops
 *           that helper's answer, then loads the scratch work object published
 *           at the scratchpad pointer cell 0x1F800044 and advances that object's
 *           unsigned byte 2, the step index the dispatcher reads, by one, so the
 *           next dispatch selects the handler at index 1 of the same run
 *           (0x800C1BD4) instead of re-entering this one. The cell is read once,
 *           after the call, and the pointer is kept across the read-modify-write
 *           rather than re-read, so this body reloads no cell. It reads or
 *           writes no other state; it takes no arguments and returns nothing,
 *           and the 0x18-byte frame only keeps the return address at 0x10 across
 *           that one call. The slot after the pointer cell load and the slot
 *           after the byte load stay nops. The roles of resource 0 and of the
 *           work record are not established here, so the name describes the
 *           observed effect and keeps the address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceZeroThenAdvanceWorkByte2BossBoss03316_800C1B9C(void) {
  func_8014D6B8(0);
  D_1F800044[2]++;
}
