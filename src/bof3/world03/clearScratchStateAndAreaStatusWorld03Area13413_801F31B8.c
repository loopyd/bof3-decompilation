#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;

/* @source 0x801F31B8
 * @behavior Overlay state handler (entry 12 of the handler table at
 * 0x801F5744, the table the target's per-frame dispatcher 0x801F2C04 indexes
 * with the work record's byte at +1): clears the state byte at +1 of the work
 * record published at the scratchpad pointer cell 0x1F800044, i.e. returns the
 * record to state 0 (whose handler 0x801F2C48 re-arms the record); then, when
 * the area status byte D_80146865 already reads its 0xFF end marker or the
 * shared mode byte D_80146866 reads 0x80, also clears the area status byte
 * itself. The pointer cell is read once and its loaded pointer is used directly
 * for the record store, while the area status byte is addressed once and that
 * one address serves both its load and its conditional store. Takes no
 * arguments, returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearScratchStateAndAreaStatusWorld03Area13413_801F31B8(void) {
  u8* status;

  SPAD_PTR_SLOT(u8, 0x44)[1] = 0;
  status = &D_80146865;
  if (*status == 0xFF || D_80146866 == 0x80) {
    *status = 0;
  }
}
