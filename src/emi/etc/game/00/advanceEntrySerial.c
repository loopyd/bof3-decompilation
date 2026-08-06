#include "internal.h"

/* @behavior waits for the streamed slot, advances the game entry and palette
 * serials, then records the next ready state.
 * @source 0x80196FFC
 * @see docs/specs/data/schema-ledger.md
 */
void advanceEntrySerial(void) {
  u16* ent;
  u16  ev;
  u8   pv;

  initStreamSlot(0x268u);

  while (!func_80162D00()) {
    func_8014B87C(1u);
  }

  stageSharedPaletteBank();

  ent = (u16*)&D_80143B90;
  ev = paletteStageSerial;
  pv = ev;
  ev = *ent;
  pv++;
  ev++;
  paletteStageSerial = pv;
  *ent = ev;
}
