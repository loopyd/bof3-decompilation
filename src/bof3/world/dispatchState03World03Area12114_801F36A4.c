#include "bof3/bof3.h"

extern void (*D_801F67F0[])(void);

/* @source 0x801F36A4
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x03 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the second local handler table
 * 0x801F67F0. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchState03World03Area12114_801F36A4(void) {
  D_801F67F0[SPAD_PTR_SLOT(u8, 0x44)[3]]();
}
