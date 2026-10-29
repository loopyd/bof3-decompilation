#include "bof3/bof3.h"

extern void (*D_801F6870[])(void);

/* @source 0x801F418C
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table
 * 0x801F6870. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchState01World03Area12114_801F418C(void) {
  D_801F6870[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
