#include "internal.h"

/* @behavior picks one target id from either the weighted global picker or the local
 * picker, then stores the chosen result byte globally.
 * @source 0x801e2948 FUN_801e2948
 */
void func_801e2948(s8 arg0) {
  volatile u8* battle_globals;
  u8           target;

  battle_globals = (volatile u8*)0x80140000;

  if ((func_8017e3d4() & 7u) < 3u) {
    target = func_801e2e30();
  } else if (battle_globals[0x62f3] == 1u) {
    target = func_801e2e30();
  } else {
    target = func_801e29b4((u8)(arg0 + 3));
  }

  battle_globals[0x6384] = target;
}
