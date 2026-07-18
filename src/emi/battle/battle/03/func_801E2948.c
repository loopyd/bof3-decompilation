#include "internal.h"

/* @behavior picks one target id from either the weighted global picker or the local
 * picker, then stores the chosen result byte globally.
 * @source 0x801E2948
 */
void func_801E2948(s8 arg0) {
  volatile u8* battle_globals;
  u8           target;

  battle_globals = (volatile u8*)0x80140000;

  if ((func_8017E3D4() & 7u) < 3u) {
    target = func_801E2E30();
  } else if (battle_globals[0x62f3] == 1u) {
    target = func_801E2E30();
  } else {
    target = func_801E29B4((u8)(arg0 + 3));
  }

  battle_globals[0x6384] = target;
}
