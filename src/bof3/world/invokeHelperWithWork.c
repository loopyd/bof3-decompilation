#include "bof3/bof3.h"

/* Helper of the shared main executable at 0x8014E4BC. The bytes prove only that
 * this body calls it once with the work record the scratchpad cell supplies;
 * its role is not established, so the callee keeps its address-anchored name.
 */
void func_8014E4BC(void* work);

/* @source 0x801F2FD4
 * @behavior Thin overlay wrapper. It loads the work-record pointer from the
 *           scratchpad pointer cell at byte offset 0x44 - cell 0x11 of the
 *           scratchpad pointer table, whose base stays at zero so the load is
 *           lui $a0,0x1F80 followed by lw $a0,0x44($a0) - and passes that
 *           pointer to the main-executable helper 0x8014E4BC as its only
 *           argument, discarding any result. Takes no arguments and returns
 *           nothing, reads and writes no other state, and its 0x18-byte frame
 *           only keeps $ra at 0x10 across that call, whose delay slot stays a
 *           nop. The cell load runs before the frame is reserved and the
 *           epilogue restores the frame ahead of the return. No jal in this
 *           payload names it; its address is entry 3 of the thirteen-word
 *           in-image code-pointer run at 0x801F4268, inside the reviewed
 *           textbin row D_801F3F10, so the overlay loader that steps that run
 *           is the demonstrated route to this body. It follows the
 *           byte-identical sibling invokeHelperWithWorkWorld02Area08513_801F3BA4
 *           of emi/world02/area085/13, whose 0x28-byte span compared equal to
 *           this one word for word, so the statement order and operand form are
 *           that sibling's and only the target-local name differs.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWithWork(void) {
  func_8014E4BC(SPAD_PTR_TABLE(u8)[0x11]);
}
