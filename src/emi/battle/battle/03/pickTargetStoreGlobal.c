#include "internal.h"

extern int rand(void);
/* @behavior picks one target id from either the weighted global picker or the local
 * picker, then stores the chosen result byte globally.
 * @source 0x801E2948
 */
void pickTargetStoreGlobal(s8 arg0) {
  u8 target;

  if ((rand() & 7) < 3) {
    goto pick_global;
  }
  if (BATTLE_GLOBAL_BYTE_62F3 == 1) {
    goto pick_global;
  }
  target = pickRandomUnblockedId((u8)(arg0 + 3));
  goto store_target;

pick_global:
  target = func_801E2E30();

store_target:
  D_80146384 = target;
}
