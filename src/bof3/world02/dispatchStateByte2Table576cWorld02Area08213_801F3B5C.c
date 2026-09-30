#include "bof3/bof3.h"

/*
 * Handler table based at 0x801F576C, the table this dispatcher selects from:
 * entry 0 = 0x801F3BA0, entry 1 = 0x801F3C7C, entry 2 = 0x801F3D04,
 * entry 3 = 0x801F3D8C, entry 4 = 0x801F3E08, entry 5 = 0x801F4008,
 * entry 6 = 0x801F4190, entry 7 = 0x801F4334 (all eight are handler
 * boundaries of this overlay). The row ends at 0x801F578C, where the byte
 * data that follows the last of them begins.
 */
extern void (*D_801F576C[])(void); /* @source 0x801F576C @kind data */

/* @source 0x801F3B5C
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table
 *           based at 0x801F576C. The selected handler's $v0 is left untouched
 *           by the epilogue, so this shim's result is whatever that handler
 *           returns. Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte2Table6284World02Area07714_801F4CDC, and the
 *           twin of this target's 0x801F49D4, which dispatches the same work
 *           byte +0x02 through the table based at 0x801F57D0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table576cWorld02Area08213_801F3B5C(void) {
  D_801F576C[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
