#include "internal.h"

extern u32 func_8014D8D4(u8 arg0);
/* @kind: table */
extern u8  game_request_remapTable[];

/* @behavior When the world/front flags bit 0 is set, remaps arg0 through
 * a 2-byte-per-entry lookup table at 0x801cd06c: calls func_8014D8D4
 * with the first table byte and writes the second byte to the scratchpad
 * work area's unk_2A field. When the flag is clear, calls func_8014D8D4
 * with arg0 directly.
 * @source 0x801B5BDC
 */
void game_request_apply_remap(u8 arg0) {
  u32 index;

  if (!(D_80143F02 & 1)) {
    func_8014D8D4(arg0);
    return;
  }
  index = (u32)arg0 * 2;
  func_8014D8D4(game_request_remapTable[index]);
  SCRATCH_WORK->unk_2A = game_request_remapTable[index + 1];
}
