#include "internal.h"

extern int rand(void);
/* @behavior picks one target id from either the weighted global picker or the local
 * picker, then stores the chosen result byte globally.
 * @source 0x801E2948
 */
void func_801E2948(s8 arg0) {
  volatile u8* battle_globals;
  u8           target;

  if ((rand() & 7u) < 3u) {
    target = func_801E2E30();
  } else if (BATTLE_GLOBAL_RAM_U8[0x62f3] == 1u) {
    target = func_801E2E30();
  } else {
    target = func_801E29B4((u8)(arg0 + 3));
  }

  BATTLE_GLOBAL_RAM_U8[0x6384] = target;
}
