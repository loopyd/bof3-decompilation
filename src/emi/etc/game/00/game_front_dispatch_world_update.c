#include "internal.h"

/* @behavior dispatches one of two world-front updates from the current
 * sub-state, then ticks the shared waiting path.
 * @source 0x80198170
 */
void game_front_dispatch_world_update(void) {
  if (D_80143B92 == 0) {
    func_80198F1C();
  } else {
    game_front_update_world_position();
  }
  func_801992B8();
}
