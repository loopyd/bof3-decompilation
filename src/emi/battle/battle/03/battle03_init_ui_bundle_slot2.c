#include "internal.h"

/* @behavior initializes one small ui state bundle with fixed halfwords and one
 * caller-provided byte.
 * @source 0x801D9388
 */
void battle03_init_ui_bundle_slot2(u8 arg0) {
  func_80158DB8(2u, 3u);
  *(u8*)0x8014837au = 1u;
  *(u16*)0x8014837cu = 0x0088u;
  *(u8*)0x8014837bu = arg0;
  *(u16*)0x8014837eu = 0x0058u;
}
