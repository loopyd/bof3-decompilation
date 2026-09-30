#include "bof3/bof3.h"

/*
 * Handler table based at 0x800C2690, the table this dispatcher selects from:
 * entry 0 = 0x801E3BAC, entry 1 = 0x801E3BD0, entry 2 = 0x801E3BAC,
 * entry 3 = 0x801E4490, entry 4 = 0x800C18E4 (this overlay), entry 5 =
 * 0x801E4928 (engine handlers).
 */
extern void (*D_800C2690[])(void); /* @source 0x800C2690 @kind data */

/* @source 0x800C18A0
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at
 * offset 0x02 as the index and calls the handler pointer at that index of the
 * local handler table based at 0x800C2690. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. Takes no arguments and no arguments are forwarded.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table2690BossBoss00816_800C18A0(void) {
  D_800C2690[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
