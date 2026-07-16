#include "internal.h"

/* @behavior submits one enemy battler script block through the common helper rooted
 * at `0x801490d8`.
 * @source 0x801DEA18
 */
void func_801DEA18(u32 arg0) {
  arg0 &= 0xff;
  arg0 = ((((arg0 << 3) + arg0) << 2) - arg0) << 3;
  func_801501E4((void*)0x801490d8u, arg0 - 0x7fe14ca4, 8u);
}
