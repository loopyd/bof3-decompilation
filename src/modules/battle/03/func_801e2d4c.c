#include "internal.h"

/* @behavior chooses between the global weighted picker and the local picker based
 * on one global mode byte.
 * @source 0x801e2d4c FUN_801e2d4c
 */
u8 func_801e2d4c(s8 arg0) {
  u8 value;

  if (((u8*)0x80140000u)[0x62f3] == 1u) {
    value = func_801e2e30();
  } else {
    value = func_801e29b4((u8)(arg0 + 3));
  }

  return value;
}
