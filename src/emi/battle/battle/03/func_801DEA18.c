#include "internal.h"

/* @behavior submits one enemy battler script block through the common helper rooted
 * at `0x801490d8`.
 * @source 0x801DEA18
 */
void func_801DEA18(u32 arg0) {
  s32 shift;
  void* data;

  arg0 &= 0xff;
  shift = 2;
  data = D_801490D8;
  func_801501E4(data,
                 (u32)(D_801EB35C + (((((arg0 << 3) + arg0) << shift) - arg0)
                                      << 3)),
                 8u);
}
