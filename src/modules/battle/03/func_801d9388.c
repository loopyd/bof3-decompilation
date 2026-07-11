#include "internal.h"

/* @behavior initializes one small ui state bundle with fixed halfwords and one
 * caller-provided byte.
 * @source 0x801d9388 FUN_801d9388
 */
void func_801d9388(u8 arg0) {
  func_80158db8(2u, 3u);
  *(u8*)0x8014837au = 1u;
  *(u16*)0x8014837cu = 0x0088u;
  *(u8*)0x8014837bu = arg0;
  *(u16*)0x8014837eu = 0x0058u;
}
