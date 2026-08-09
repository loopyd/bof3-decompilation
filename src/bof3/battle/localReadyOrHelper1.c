#include "bof3/battle/battle03_internal.h"

/* @behavior returns ready immediately when the local work flags allow it; otherwise
 * delegates to the first EXE-side readiness helper.
 * @source 0x801DEDE4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u8 localReadyOrHelper1(void) {
  volatile Battle03LocalWork* battle_work = BATTLE_LOCAL_WORK_PTR;
  volatile u8*                battle_global = BATTLE_GLOBAL_RAM_U8;

  if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 4u) != 0u) {
    return 1u;
  }

  if ((battle_global[0x63ceu] != 0u) &&
      ((BATTLE_LOCAL_WORD_128(battle_work) & 0x10u) == 0u)) {
    return 1u;
  }

  return func_8014D978();
}
