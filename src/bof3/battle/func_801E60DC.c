#include "bof3/battle/battle03_internal.h"

/* @source 0x801E60DC
 * @behavior Advances the current queued slot motion and resets it when its timer expires.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801E60DC(void) {
  /* MATCHING_AID: clean-C, profile, and permuter rungs left an exact-size
   * entry allocator residual (slot a1 versus v1); pin only the slot lifetime.
   * Remove when the canonical compiler allocates this pointer to v1 unaided. */
  REGISTER_PIN(Battle03QueuedSlot*, slot, "v1");
  s32 value_3a;
  s32 value_36;

  slot = D_801EC2E0;
  slot->unk_38 += 0x40000;
  if (slot->unk_09-- == 0u) {
    slot = D_801EC2E0;
    slot->unk_01++;

    slot = D_801EC2E0;
    value_3a = FIELD_REF(s16, slot, 0x3a);
    value_36 = FIELD_REF(s16, slot, 0x36);
    slot->unk_0c = 0x8000;
    slot->unk_10 = -0x60000;
    slot->unk_1c = value_3a;
    slot->unk_18 = value_36;
  }
}
