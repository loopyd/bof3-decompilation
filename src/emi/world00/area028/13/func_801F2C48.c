#include "internal.h"

/* @behavior seeds the work table, then increments work byte one and clears
 * the halfword at offset 0x2E of the current scratchpad work record.
 * @source 0x801F2C48
 */
void func_801F2C48(void) {
  Area028SpriteSlot* slot;

  seedWorkTable();
  slot = SPAD_PTR_SLOT(Area028SpriteSlot, 0x44u);
  slot->unk_01++;
  slot->unk_2e = 0;
}
