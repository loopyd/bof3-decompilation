#include "bof3/bof3.h"

extern void (*D_801F6894[])(void);

/* @source 0x801F45E0
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table
 * 0x801F6894. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table6894World03Area12114_801F45E0(void) {
  D_801F6894[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
