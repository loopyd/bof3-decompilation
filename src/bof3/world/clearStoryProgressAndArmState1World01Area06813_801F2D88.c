#include "bof3/bof3.h"

/* @source 0x80146865 @kind data */
extern u8 D_80146865;

/* @source 0x801F2D88
 * @behavior Overlay state handler, entry 3 of the handler table at 0x801F3FF4
 *           that the local dispatcher
 *           dispatchStateByte1Table3ff4World01Area06813 selects from with the
 *           work record's byte at +1: clears the shared story-progress byte
 *           D_80146865, then arms that state byte of the work record published
 *           at the scratchpad pointer cell 0x1F800044 with 1, the index of the
 *           handler func_801F2C6C. The pointer cell is read once and that
 *           pointer serves the state store, which fills the return delay slot,
 *           so the shared byte is cleared first. Takes no arguments, reads no
 *           other state and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStoryProgressAndArmState1World01Area06813_801F2D88(void) {
  u8* record;

  record = SPAD_PTR_SLOT(u8, 0x44);
  D_80146865 = 0;
  record[1] = 1;
}
