#include "internal.h"

/* @behavior dispatches through the indexed handler table at D_801C84BC
 * using the s8 state selector at D_801448EB.
 * @source 0x801A7CAC
 */
void func_801A7CAC(void) {
  D_801C84BC[(s32)D_801448EB]();
}
