#include "internal.h"

extern u8 D_80140000[];

#define EMI_LOADER_STEP (*(volatile u32*)(D_80140000 + 0x646c))

extern vu8            D_80146483;
extern vu32           D_80146458;
extern EmiLoaderEntry D_8014677C[];

/* @behavior selects the current EMI entry's primary destination, advances the
 * loader state machine, and advances the loader step.
 * @source 0x801629f0 FUN_801629f0
 */
void func_801629f0(void) {
  if (EMI_LOADER_STEP == 0) {
    D_80146458 = D_8014677C[D_80146483].destination;
  }

  func_80162c14();
  EMI_LOADER_STEP += 1;
}
