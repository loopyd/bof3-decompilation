#include "internal.h"

/* @source 0x801EA530
 * @behavior subtracts the current panel counter from its byte budget, clearing
 * the budget and companion state when the counter has caught up.
 */
void consumePanelBudget(u8* arg0, u32 arg1, u32 arg2, u32 arg3, s16* arg4,
                   u8* arg5, s16* arg6, u8* arg7) {
  s16 counter;
  u8  budget;

  counter = *arg6;
  budget = *arg5;
  if (counter < (s32)budget) {
    *arg5 = budget - counter;
  } else {
    *arg5 = 0u;
    *arg7 = 0u;
  }
  if ((*arg4 != 0) && (*arg0 == 0u)) {
    *arg0 = 1u;
  }
}
