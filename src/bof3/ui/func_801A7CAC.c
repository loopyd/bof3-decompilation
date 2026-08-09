#include "bof3/ui/game00_internal.h"

/* @behavior dispatches through the indexed handler table at D_801C84BC
 * using the s8 state selector at D_801448EB.
 * @source 0x801A7CAC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801A7CAC(void) {
  D_801C84BC[(s32)D_801448EB]();
}
