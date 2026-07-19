#include "internal.h"

/* @behavior chooses between the global weighted picker and the local picker based
 * on one global mode byte.
 * @source 0x801E2D4C
 */
u8 func_801E2D4C(s8 arg0) {
  u8 value;

  if (BATTLE_GLOBAL_BYTE_62F3 == 1u) {
    value = func_801E2E30();
  } else {
    value = func_801E29B4((u8)(arg0 + 3));
  }

  return value;
}
