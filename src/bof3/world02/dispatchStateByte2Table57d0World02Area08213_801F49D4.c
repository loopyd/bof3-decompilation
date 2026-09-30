#include "bof3/bof3.h"

/*
 * Handler table based at 0x801F57D0, the table this dispatcher selects from:
 * entry 0 = 0x801F4A18, entry 1 = 0x801F4A90, entry 2 = 0x801F4AFC,
 * entry 3 = 0x801F4B3C, entry 4 = 0x801F4C34, entry 5 = 0x801F4C90,
 * entry 6 = 0x801F4D6C, entry 7 = 0x801F4DC8, entry 8 = 0x801F4F28,
 * entry 9 = 0x801F4F88, entry 10 = 0x801F5068, entry 11 = 0x801F50C4,
 * entry 12 = 0x801F5124, entry 13 = 0x801F51BC, entry 14 = 0x801F527C,
 * entry 15 = 0x801F52D8 (all sixteen are handler boundaries of this overlay;
 * entry 2 is this target's already-lifted 0x801F4AFC palette handler). The
 * row ends at 0x801F5810, where the following byte data begins.
 */
extern void (*D_801F57D0[])(void); /* @source 0x801F57D0 @kind data */

/* @source 0x801F49D4
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table
 *           based at 0x801F57D0. The selected handler's $v0 is left untouched
 *           by the epilogue, so this shim's result is whatever that handler
 *           returns. Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte2Table6284World02Area07714_801F4CDC, and the
 *           twin of this target's 0x801F3B5C, which dispatches the same work
 *           byte +0x02 through the table based at 0x801F576C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table57d0World02Area08213_801F49D4(void) {
  D_801F57D0[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
