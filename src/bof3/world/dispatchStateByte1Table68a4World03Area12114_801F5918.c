#include "bof3/bof3.h"

extern void (*D_801F68A4[])(void);

/* @source 0x801F5918
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the second local handler table
 * 0x801F68A4. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table68a4World03Area12114_801F5918(void) {
  D_801F68A4[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
