#include "bof3/world/area00813_internal.h"

extern void func_8014D6B8(u32 flag);
extern void func_80196070(void);

/* @source 0x801F31D0
 * @behavior installs the local area state as both the current state and the
 *           scratchpad state, requests resource 1 through the shared area
 *           resource selector, clears bit 0x40 in the current state byte at
 *           offset 0x118, restores the previous scratchpad state pointer and
 *           finally calls the shared 0x80196070 work-area reset helper.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F31D0(void) {
  volatile World00Area008State* previous;

  previous = g_areaWork;
  currentState = &areaState;
  g_areaWork = &areaState;
  func_8014D6B8(1);
  currentState->unk_118 &= 0xBFu;
  g_areaWork = previous;
  func_80196070();
}
