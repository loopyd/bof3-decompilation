#include "bof3/bof3.h"

/* @source 0x801F46E4
 * @behavior Overlay step handler: entry 0 of the three-word handler run based
 * at 0x801F4AC4, the table this target's exact dispatcher
 * dispatchStateByte1Table4ac4World03Area14013_801F46A0 indexes with the work
 * record's state byte at +0x01. Stores 210 (0xD2) into the scratchpad work
 * object's countdown byte at +0x09 published through the pointer cell at
 * 0x1F800044, then advances that object's state byte at +0x01 to 1, re-reading
 * the pointer cell with its own `lui`/`lw` pair for the second store so the next
 * frame dispatches entry 1. Takes no arguments and returns nothing; no other
 * state is read or written.
 *
 * The next handler, func_801F4708 at entry 1, decrements the same countdown
 * byte and advances the state byte to 2 when it expires, which identifies +0x09
 * as a frame countdown armed here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownByte9To210AndAdvanceWorkByte1World03Area14013_801F46E4(void) {
  u8** slots;

  slots = SPAD_PTR_TABLE(u8);
  slots[0x11][9] = 210;
  slots[0x11][1] = 1;
}
