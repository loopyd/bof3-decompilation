#include "internal.h"

/* @behavior waits for the streamed slot, advances the game entry and palette
 * serials, then records the next ready state.
 * @source 0x80196FFC
 * @see docs/specs/data/schema-ledger.md
 */
void game_front_advance_entry_serial(void) {
  u16* ent;
  u16  ev;
  u8   pv;

  emi_stream_init_slot(0x268u);

  while (!func_80162D00()) {
    func_8014B87C(1u);
  }

  game_stage_shared_palette_bank();

  ent = (u16*)&D_80143B90;
  ev = GAME_FRONT_PALETTE_STAGE_SERIAL;
  pv = ev;
  ev = *ent;
  pv++;
  ev++;
  GAME_FRONT_PALETTE_STAGE_SERIAL = pv;
  *ent = ev;
}
