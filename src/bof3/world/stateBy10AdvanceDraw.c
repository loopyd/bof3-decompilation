#include "bof3/bof3.h"

void stateWorkWhen454Draw(
    void);

/* @source 0x801F3560
 * @behavior Advances the work halfword at offset 0x2E, published through the
 *           scratchpad pointer cell 0x1F800044, by 0x10 and stores the
 *           truncated halfword. When that stored signed value is at least
 *           0x10 it increments work state byte 0x02, keeping the pointer read
 *           once before both record accesses. Then runs the state-2 handler
 *           0x801F35BC, which draws that halfword. Takes no arguments, uses no
 *           result and saves only the return address. The 0x5C bytes are
 *           byte-identical to the exact
 *           advanceHalfword2eBy10AndAdvanceState2At10ThenDrawHalfword2eWorld03Area12114_801F3574
 *           in emi/world03/area121/14 apart from the low byte of the internal
 *           `jal` word that encodes that overlay's own state-2 handler, so this
 *           body follows the reviewed sibling statement order with this
 *           target's handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateBy10AdvanceDraw(
    void) {
  u8* cursor;
  s16* half;

  cursor = SPAD_PTR_SLOT(u8, 0x44);
  half = (s16*)(cursor + 0x2E);
  *half = (s16)(*half + 0x10);
  if (*half >= 0x10) {
    cursor[2]++;
  }
  stateWorkWhen454Draw();
}
