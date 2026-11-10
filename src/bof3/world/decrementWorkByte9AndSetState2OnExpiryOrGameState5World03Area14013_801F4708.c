#include "bof3/bof3.h"

/* @source 0x80143BB0 @kind data */
extern u8 D_80143BB0;

/* @source 0x801F4708
 * @behavior Entry 1 of the handler table at 0x801F4AC4. Decrements the
 * scratchpad work record's countdown byte at +0x09, wrapping as an unsigned
 * byte, and sets state byte +0x01 to 2 when it reaches zero. Also sets that
 * state to 2 when the game-state byte at 0x80143BB0 is 5. Each state store
 * re-reads the work pointer cell at 0x1F800044. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndSetState2OnExpiryOrGameState5World03Area14013_801F4708(void) {
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);
  if (--slots[0x11][9] == 0) {
    slots[0x11][1] = 2;
  }
  if (D_80143BB0 == 5) {
    slots[0x11][1] = 2;
  }
}
