#include "bof3/ui/game00_internal.h"

/* @source 0x801C1F4C
 * @behavior for each of the eight stride-8 front-end record slots whose kind
 * byte at D_801457A8 equals arg0 and whose flags byte at D_801457AB is
 * nonzero, counts the active records at D_801455C8/D_801455C9 claimed by that
 * slot (owning slot byte == slot + 1) and, when no record claims the slot,
 * increments that slot's counter word at D_801457AC, the state-block word at
 * D_801448D8 + 0xED4 + 8 * slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801C1F4C(u8 arg0) {
  u8  count;
  s32 slot;
  s32 offset;
  s32 record;
  s32 owner;
  s32 counter;
  u8* state;

  for (slot = 0; slot < 8; slot++) {
    offset = slot * 8;
    state = D_801448D8 + offset;
    count = 0;
    if (D_801457A8[offset] == arg0 && D_801457AB[offset] != 0) {
      owner = slot + 1;
      for (record = 0; record < 0x1E0; record += 8) {
        if (D_801455C8[record] != 0 && D_801455C9[record] == owner) {
          count++;
        }
      }
      if (count == 0) {
        /* The original object reads the counter through the byte-offset view
         * and republishes it through the state-block cursor. */
        counter = FIELD_REF(s32, D_801457AC, offset);
        FIELD_REF(s32, state, 0xED4) = counter + 1;
      }
    }
  }
}
