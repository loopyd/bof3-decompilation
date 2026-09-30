#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE43C
 * @behavior Clears the dispatched-handler mask at 0x80146329, then scans the
 * eight 12-byte battle dispatch slots twice, once per pass: for every occupied
 * slot whose byte +0x03 equals the pass and whose byte +0x01 does not
 * intersect that mask, it publishes the slot pointer to 0x801EB4DC, stores the
 * slot index in scratchpad byte 0x00, and invokes the slot's handler from the
 * five-entry table copied off 0x801D0C84, indexed by the slot's byte +0x01.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 73/73 instructions, 292 bytes, live byte match.
 */
void func_801DE43C(void) {
  Battle03FiveDispatchTable handlers;
  u8*                       slot;
  u8                        pass;
  s8                        index;

  handlers = D_801D0C84;
  pass = 0;
  D_80146329 = 0;

  for (pass = 0; pass < 2; pass++) {
    for (index = 0; index < 8; index++) {
      slot = (u8*)&battleDispatchSlots[index * 12];
      D_801EB4DC = slot;
      if (slot[0] == 0) {
        continue;
      }
      if (slot[3] != pass) {
        continue;
      }
      if ((slot[1] & D_80146329) != 0) {
        continue;
      }
      SPAD_REF(u8, 0) = index;
      handlers.handlers[slot[1]]();
    }
  }
}
