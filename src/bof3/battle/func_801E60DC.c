#include "bof3/battle/battle03_internal.h"

/* @source 0x801E60DC
 * @behavior Advances the current queued slot vertical motion word +0x38 by
 * 0x40000 and counts down its timer byte +0x09; once that byte expires it
 * increments the slot counter byte +0x01 and resets the motion: the delta pair
 * +0x0C/+0x10 becomes 0x8000/-0x60000 and the limit word +0x1C and origin word
 * +0x18 take the slot halfwords +0x3A and +0x36 that advanceSlotPosition
 * compares against them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E60DC(void) {
  Battle03QueuedSlot* slot;
  Battle03QueuedSlot* expired;

  slot = D_801EC2E0;
  slot->unk_38 += 0x40000;
  if (slot->unk_09-- == 0) {
    D_801EC2E0->unk_01++;
    expired = D_801EC2E0;
    expired->unk_0c = 0x8000;
    expired->unk_10 = -0x60000;
    expired->unk_1c = FIELD_REF(s16, expired, 0x3a);
    expired->unk_18 = FIELD_REF(s16, expired, 0x36);
  }
}
