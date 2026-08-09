#include "bof3/battle/battle03_internal.h"

/* @behavior chooses between the global weighted picker and the local picker based
 * on one global mode byte.
 * @source 0x801E2D4C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 pickTargetByMode(s8 arg0) {
  u8 value;

  if (BATTLE_GLOBAL_BYTE_62F3 == 1u) {
    value = func_801E2E30();
  } else {
    value = pickRandomUnblockedId((u8)(arg0 + 3));
  }

  return value;
}
