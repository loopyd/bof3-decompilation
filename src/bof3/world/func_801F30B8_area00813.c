#include "bof3/world/area00813_internal.h"

#include <rand.h>

extern void func_8014D6B8(u32 flag);

/* @source 0x801F30B8
 * @behavior selects area mode 9 when the shared high-bit flag is set;
 *           otherwise installs the local area state as both the current state
 *           and the scratchpad state and, when the shared area state halfword
 *           reads 8 while the shared state byte 0x8014601A reads 1, requests
 *           the shared area resource selector with argument 1, stores the
 *           random mode-table byte into the shared countdown, restores the
 *           previous scratchpad state pointer and selects mode 8 on it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F30B8(void) {
  volatile World00Area008State* previous;

  if ((D_80146867 & 0x80u) != 0u) {
    g_areaWork->mode = 9;
    return;
  }

  previous = g_areaWork;
  currentState = &areaState;
  g_areaWork = &areaState;

  if (D_80146028 == 8) {
    if (D_8014601A == 1) {
      func_8014D6B8(1);
      countdown = D_801F4680[rand() & 3];
      g_areaWork = previous;
      previous->mode = 8;
    }
  }
}
