#include "internal.h"

/* @behavior chooses between the global weighted picker and the local picker based
 * on one global mode byte.
 * @source 0x801E2D4C
 */
u8 battle03_pick_target_by_mode(s8 arg0) {
  u8 value;

  if (BATTLE_GLOBAL_BYTE_62F3 == 1u) {
    value = func_801E2E30();
  } else {
    value = battle03_pick_random_unblocked_id((u8)(arg0 + 3));
  }

  return value;
}
