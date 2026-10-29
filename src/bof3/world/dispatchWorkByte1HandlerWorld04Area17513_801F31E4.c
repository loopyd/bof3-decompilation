#include "bof3/bof3.h"

extern void (*D_801F45F8[])(void);

/* @source 0x801F31E4
 * @behavior Reads the non-volatile scratchpad work-object pointer cell at
 * 0x1F800044, indexes the five-entry handler table D_801F45F8 at 0x801F45F8
 * with that object's dispatch byte at offset 0x01 (scaled by four) and calls
 * the selected handler; takes no arguments, returns nothing, and reads or
 * writes no other work object byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerWorld04Area17513_801F31E4(void) {
  D_801F45F8[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
