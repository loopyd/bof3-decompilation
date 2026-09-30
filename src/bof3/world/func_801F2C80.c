#include "bof3/world/area02813_internal.h"

/* @behavior gates on the scenario progress byte: at progress 0x19 it increments
 * the byte at offset 1 of the record held in scratchpad cell 0x1F800044;
 * otherwise it hands that record's 0x2E halfword to func_801F318C, refreshes
 * the sprite slots, then advances the halfword by 0x40 and clamps it to 0x400.
 * @source 0x801F2C80
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2C80(void) {
  Area028SpriteSlot* slot;

  if (g_ScenarioProgress == 0x19) {
    slot = D_1F800044;
    slot->unk_01++;
  } else {
    func_801F318C(D_1F800044->unk_2e);
    refreshSpriteSlots();
    slot = D_1F800044;
    slot->unk_2e += 0x40;
    if (slot->unk_2e >= 0x400) {
      slot->unk_2e = 0x400;
    }
  }
}
