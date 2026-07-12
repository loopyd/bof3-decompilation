#include "internal.h"

/* @behavior resets the current scratch object's four state bytes, clears one
 * pending bit using byte `5`, and conditionally clears byte `0x119` on the
 * current local work when the global `0x40` flag is absent.
 * @source 0x801e1dd4 FUN_801e1dd4
 */
void func_801e1dd4(void) {
  BATTLE_LOCAL_SCRATCH_PTR->unk_01 = 2u;
  BATTLE_LOCAL_SCRATCH_PTR->unk_02 = 0u;
  BATTLE_LOCAL_SCRATCH_PTR->unk_03 = 0u;
  BATTLE_LOCAL_SCRATCH_PTR->unk_04 = 0u;
  func_801de1b0(BATTLE_LOCAL_BYTE_05(BATTLE_LOCAL_SCRATCH_PTR));
  BATTLE_LOCAL_WORD_124(BATTLE_LOCAL_WORK_PTR) &= 0xfffffdffu;
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x40u) == 0u) {
    BATTLE_LOCAL_BYTE_119(BATTLE_LOCAL_WORK_PTR) = 0u;
  }
}
