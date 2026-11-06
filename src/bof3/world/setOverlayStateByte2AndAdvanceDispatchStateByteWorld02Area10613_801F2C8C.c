#include "bof3/bof3.h"

extern u8 D_8014933B;

/* @source 0x801F2C8C
 * @behavior Overlay step handler, entry 0 of the state dispatch table at
 *           0x801F3EAC that the state-byte-+2 dispatcher 0x801F2C48 indexes:
 *           writes the constant 2 to the shared overlay state byte at
 *           0x8014933B, then advances the scratch work record's dispatch state
 *           byte +0x02 by one, handing the next frame to entry 1 of the same
 *           table. The record pointer is published through the scratchpad
 *           pointer cell 0x1F800044 and is loaded once for both field
 *           accesses. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setOverlayStateByte2AndAdvanceDispatchStateByteWorld02Area10613_801F2C8C(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  D_8014933B = 0x02;
  work[2]++;
}
