#include "bof3/bof3.h"

extern void (*D_801F67B4[])(void);

/* @source 0x801F2C3C
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x04 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table
 * 0x801F67B4. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchState04World03Area12114_801F2C3C(void) {
  D_801F67B4[SPAD_PTR_SLOT(u8, 0x44)[4]]();
}
