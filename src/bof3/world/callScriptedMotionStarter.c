#include "bof3/bof3.h"

/* @source 0x8015D554 @kind function */
extern void func_8015D554(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F378C
 * @behavior Calls the shared executable front-end scripted-motion starter at
 * 0x8015D554 with arguments 0, 1, 4 and 0x1E, then returns. The overlay
 * registers this body as one entry of its handler pointer run at 0x801F4FD0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callScriptedMotionStarter(void) {
  func_8015D554(0u, 1u, 4u, 0x1E);
}
