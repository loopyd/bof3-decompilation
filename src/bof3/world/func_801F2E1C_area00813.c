#include "bof3/world/area00813_internal.h"

#include <rand.h>

extern void func_8014D6B8(u32 flag);

/* @source 0x801F2E1C
 * @behavior selects area mode 9 when the shared high-bit flag is set; selects
 *           mode 6 and restarts the shared secondary state bytes when the
 *           shared state byte 0x80146865 reads 1; otherwise decrements the
 *           shared countdown byte and, when the decremented value is below 2,
 *           clears the shared secondary state byte, installs the local area
 *           state as both the current state and the scratchpad state, requests
 *           the shared area resource selector with argument 1, stores the
 *           random countdown-table byte into the shared countdown, restores the
 *           previous scratchpad state pointer and selects mode 4 on it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2E1C(void) {
  volatile World00Area008State* previous;
  u8 remaining;

  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9;
    return;
  }

  if (D_80146865 == 1) {
    D_80146866 = 5;
    D_80146865 = 2;
    g_areaWork->mode = 6;
    return;
  }

  remaining = (u8)(countdown - 1u);
  countdown = remaining;
  if (remaining < 2u) {
    D_80146866 = 2;
    previous = g_areaWork;
    currentState = &areaState;
    g_areaWork = &areaState;
    func_8014D6B8(1);
    countdown = D_801F4680[rand() & 3];
    g_areaWork = previous;
    previous->mode = 4;
  }
}
