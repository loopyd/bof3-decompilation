#include "internal.h"

/* does: resets the current local work record's transient fields, copies the
 * current global selector byte, then stores the shared helper result.
 * @source: 0x801def0c FUN_801def0c
 */
void func_801def0c(void) {
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_29 = 4u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_0c = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_10 = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_14 = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_18 = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_1c = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_20 = 0;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_08 = BOF3_BATTLE_LOCAL_BYTE_62EC;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_48 = 0u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_3e =
      func_8015477c(BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_34,
                    BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_38);
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_2b = 0u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_01 = 2u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_02 = 0u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_03 = 0u;
  BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_04 = 0u;
}
