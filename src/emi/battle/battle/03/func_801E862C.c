#include "internal.h"

/* @behavior dispatches the current result-ui aux state byte through one of two
 * fixed handlers.
 * @source 0x801E862C
 */
void func_801E862C(void) {
  Battle03Handler table[2];
  volatile Battle03LocalWork* work;

  do {
    work = BATTLE_LOCAL_SCRATCH_PTR;
  } while (0);
  table[0] = func_801E8684;
  table[1] = func_801E8D04;
  table[work->unk_01]();
}
