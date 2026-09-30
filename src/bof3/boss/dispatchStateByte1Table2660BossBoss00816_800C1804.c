#include "bof3/bof3.h"

/*
 * Handler table based at 0x800C2660, the table this dispatcher selects from:
 * entry 0 = 0x800C1848 (this overlay's setup handler), entries 1-5 =
 * 0x801E52E8/0x801E35D8/0x801E3638/0x801E3980/0x801E3A00 (engine handlers),
 * entry 6 = 0x800C18A0 (this overlay's byte-2 dispatcher).
 */
extern void (*D_800C2660[])(void); /* @source 0x800C2660 @kind data */

/* @source 0x800C1804
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x01 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2660. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Same shape as the already-exact sibling
 * dispatchStateByte1Table6268World02Area07714_801F3E04. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table2660BossBoss00816_800C1804(void) {
  D_800C2660[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
