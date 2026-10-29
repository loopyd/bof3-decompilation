#include "bof3/bof3.h"

extern void (*D_801F67E0[])(void);

/* @source 0x801F34E0
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x02 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table
 * 0x801F67E0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchState02World03Area12114_801F34E0(void) {
  D_801F67E0[SPAD_PTR_SLOT(u8, 0x44)[2]]();
}
