#include "bof3/battle/battle03_internal.h"

/* @behavior Rolls the current queued slot's digit counter (halfword +0x32) forward
 * by one modulo 10, then redraws its digit run: for the slot digit index byte +0x0A
 * counted down to 0 it forwards the recomputed halfword +0x36, the signed halfword
 * +0x3A, the caller delay byte and the rolled digit to func_801D9684.
 * @source 0x801E62BC
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E62BC(u8 arg0) {
  Battle03QueuedSlot* slot;
  Battle03QueuedSlot* entry;
  s16                 counter;
  s8                  digit_index;

  slot = D_801EC2E0;
  counter = (s16)(FIELD_REF(u16, slot, 0x32u) + 1);
  FIELD_REF(s16, slot, 0x32u) = counter % 10;
  for (digit_index = FIELD_REF(s8, slot, 0x0au); digit_index >= 0;
       digit_index -= 1) {
    entry = D_801EC2E0;
    func_801D9684((s16)(FIELD_REF(u16, entry, 0x36u) - (digit_index * 8) - 0x0cu),
                  FIELD_REF(s16, entry, 0x3au), arg0,
                  (u16)((FIELD_REF(s16, entry, 0x32u) + digit_index) % 10));
  }
}
