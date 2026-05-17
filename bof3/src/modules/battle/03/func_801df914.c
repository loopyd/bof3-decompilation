#include "internal.h"

/* does: writes the pair of local bytes selected from one of two tables, emits a
 * shared effect id, reruns the common readiness helper, then advances the local
 * state byte.
 * @source: 0x801df914 FUN_801df914
 */
void func_801df914(void) {
  u32 value;

  if ((BATTLE_LOCAL_WORD_128(BATTLE_LOCAL_WORK_PTR) & 2u) != 0u) {
    value = BATTLE_LOCAL_BYTE_TABLE_0198[BATTLE_GLOBAL_BYTE_63C9];
  } else {
    value = BATTLE_LOCAL_BYTE_TABLE_018C[BATTLE_LOCAL_BYTE_79(
        BATTLE_LOCAL_WORK_PTR)];
  }

  BATTLE_LOCAL_BYTE_09(BATTLE_LOCAL_SCRATCH_PTR) = value;
  BATTLE_LOCAL_BYTE_0A(BATTLE_LOCAL_SCRATCH_PTR) = value;
  func_8014d8d4(BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x0cu);
  func_801dede4();
  BATTLE_LOCAL_SCRATCH_PTR->unk_01 =
      BATTLE_LOCAL_SCRATCH_PTR->unk_01 + 1u;
}
