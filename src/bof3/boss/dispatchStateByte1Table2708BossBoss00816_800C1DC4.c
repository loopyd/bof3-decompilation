#include "bof3/bof3.h"

/*
 * Handler table based at 0x800C2708, the table this dispatcher selects from:
 * entry 0 = 0x800C1E08 (this overlay's byte-1 setup handler), entries 1-11 =
 * engine handlers 0x801E52E8/0x801E35D8/0x801E3638/0x801E3980/0x801E3A00/
 * 0x800C18A0 (this overlay's byte-2 dispatcher)/0x801E4AE8/0x801E4D8C/
 * 0x801E4F64/0x801E52E8/0x801E5288.
 */
extern void (*D_800C2708[])(void); /* @source 0x800C2708 @kind data */

/* @source 0x800C1DC4
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x01 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2708. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Same shape as the already-exact sibling
 * dispatchStateByte1Table2660BossBoss00816_800C1804, which reads the same
 * dispatch byte through the table based at 0x800C2660. Takes no arguments and
 * forwards none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table2708BossBoss00816_800C1DC4(void) {
  D_800C2708[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
