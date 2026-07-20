#include "internal.h"

/* @behavior waits for the streamed slot, advances the game entry and palette
 * serials, then records the next ready state.
 * @source 0x80196FFC
 * @see docs/specs/data/schema-ledger.md
 */
void func_80196FFC(void) {
  u16* ent;
  u16  ev;
  u8   pv;

  emi_stream_init_slot(0x268u);

  while (!emi_loader_is_ready()) {
    func_8014B87C(1u);
  }

  func_8014E284();

  ent = (u16*)&D_80143B90;
  ev = D_80145988;
  pv = ev;
  ev = *ent;
  pv++;
  ev++;
  D_80145988 = pv;
  *ent = ev;
}
