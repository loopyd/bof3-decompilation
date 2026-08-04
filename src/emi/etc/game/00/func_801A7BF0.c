#include "internal.h"

extern s8                   D_801448EA;
extern GameEntry0StateHandler D_801C84A4[];

/* @behavior dispatches through the indexed handler table at D_801C84A4
 * using the s8 state selector at D_801448EA.
 * @source 0x801A7BF0
 */
void func_801A7BF0(void) {
  D_801C84A4[(s32)D_801448EA]();
}
