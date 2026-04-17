#include "internal.h"

/* does: dispatches the current local substate-3 byte through its table.
 * @source: 0x801e046c FUN_801e046c
 */
void BOF3_NO_SIBLING_CALLS func_801e046c(void) {
  (*(Battle03Handler const volatile*)((volatile u8*)0x801f0000u +
                                      ((u32)
                                           BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_03
                                       << 2) -
                                      0x4e20u))();
}
