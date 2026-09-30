#include "bof3/bof3.h"

/*
 * Handler table based at 0x801F6284, the table this dispatcher selects from:
 * entry 0 = 0x801F4D20, entry 1 = 0x801F4DF8, entry 2 = 0x801F4EC4,
 * entry 3 = 0x801F4FFC, entry 4 = 0x801F515C (all five are handler boundaries
 * of this overlay). The row ends at 0x801F6298, where the overlay's byte data
 * begins.
 */
extern void (*D_801F6284[])(void); /* @source 0x801F6284 @kind data */

/* @source 0x801F4CDC
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table based
 *           at 0x801F6284. The selected handler's $v0 is left untouched by the
 *           epilogue, so this shim's result is whatever that handler returns.
 *           Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte1Table6268World02Area07714_801F3E04, which
 *           dispatches work byte +0x01 through the table based at 0x801F6268,
 *           and the twin of this target's 0x801F52F8, which dispatches the
 *           same work byte +0x02 through the table based at 0x801F62A8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table6284World02Area07714_801F4CDC(void) {
  D_801F6284[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
