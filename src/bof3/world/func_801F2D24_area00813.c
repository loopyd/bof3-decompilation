#include "bof3/world/area00813_internal.h"

#include <rand.h>

/* @source 0x801F2D24
 * @behavior selects area mode 9 when the shared high-bit flag is set; selects
 *           mode 6 and restarts the shared secondary state bytes when the
 *           shared state byte 0x80146865 reads 1; otherwise installs the
 *           local area state as both the current state and the scratchpad
 *           state, and when the shared area state halfword reads 4 while the
 *           shared state byte 0x8014601A reads 1, stores the random
 *           mode-table byte into the shared countdown, restores the previous
 *           scratchpad state pointer and selects mode 3 on it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2D24(void) {
  volatile World00Area008State* previous;

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

  previous = g_areaWork;
  currentState = &areaState;
  g_areaWork = &areaState;

  if (D_80146028 == 4) {
    if (D_8014601A == 1) {
      countdown = D_801F4674[rand() & 7];
      g_areaWork = previous;
      previous->mode = 3;
    }
  }
}
