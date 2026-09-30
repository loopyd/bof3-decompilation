#include "bof3/bof3.h"

extern void (*D_801F460C[])(void);

/* @source 0x801F352C
 * @behavior Reads the non-volatile scratchpad work-object pointer cell at
 * 0x1F800044 and calls the handler pointer selected by that object's dispatch
 * byte at offset 0x01 in the overlay's four-entry handler table D_801F460C at
 * 0x801F460C (entries 0x801F3570, 0x801F35B4, 0x801F3644 and 0x801F36BC);
 * takes no arguments, returns nothing and reads or writes no other
 * work-object byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1Table460cWorld04Area17513_801F352C(void) {
  D_801F460C[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
