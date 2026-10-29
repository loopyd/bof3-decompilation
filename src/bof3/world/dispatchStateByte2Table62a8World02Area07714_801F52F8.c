#include "bof3/bof3.h"

/*
 * Handler table based at 0x801F62A8, the table this dispatcher selects from:
 * entry 0 = 0x801F533C, entry 1 = 0x801F5374, entry 2 = 0x801F53BC,
 * entry 3 = 0x801F541C, entry 4 = 0x801F5464, entry 5 = 0x801F564C,
 * entry 6 = 0x801F56C4, entry 7 = 0x801F5710, entry 8 = 0x801F57BC (all nine
 * are handler boundaries of this overlay). The row ends at 0x801F62CC, where
 * the overlay's word data begins.
 */
extern void (*D_801F62A8[])(void); /* @source 0x801F62A8 @kind data */

/* @source 0x801F52F8
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table based
 *           at 0x801F62A8. The selected handler's $v0 is left untouched by the
 *           epilogue, so this shim's result is whatever that handler returns.
 *           Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte1Table6270World02Area07714_801F4034, which
 *           dispatches work byte +0x01 through the table based at 0x801F6270,
 *           and the twin of this target's 0x801F4CDC, which dispatches the
 *           same work byte +0x02 through the table based at 0x801F6284.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table62a8World02Area07714_801F52F8(void) {
  D_801F62A8[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
