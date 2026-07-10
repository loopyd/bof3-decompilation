#include "internal.h"

/* does: selects one of two local-work-driven effect ids based on flag `0x800`
 * and forwards the result to the shared EXE-side helper.
 * @source: 0x801df34c FUN_801df34c
 */
void func_801df34c(void) {
  func_8014d8d4((BATTLE_LOCAL_FLAGS_80(BATTLE_LOCAL_WORK_PTR) & 0x0800u) != 0u
                    ? BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x34u
                    : BATTLE_LOCAL_SCRATCH_PTR->unk_08 + 0x10u);
}
