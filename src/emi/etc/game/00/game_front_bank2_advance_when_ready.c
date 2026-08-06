#include "internal.h"

/* @behavior advances the second frontend callback bank after its resource,
 * update, and local gate are all ready.
 * @source 0x80197C1C
 */
void game_front_bank2_advance_when_ready(void) {
  u8   local_ready;
  u16* state;

  local_ready = func_801BF11C();
  func_801A06D8();
  func_801992B8();
  if (func_80162D00() && local_ready && D_80149332 == 0) {
    state = &D_80143B92;
    (*state)++;
  }
}
