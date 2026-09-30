#include "bof3/world/area03004_internal.h"

/* @behavior submits the AREA030 panel for the requested rectangle, then the
 * inset frame rectangle offset two pixels in and five pixels smaller; the
 * trailing byte selects the frame draw mode.
 * @source 0x801D9534
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D9534(s16 arg0, u16 arg1, s16 arg2, s16 arg3, u8 arg4) {
  func_801D934C(arg0, arg1, arg2, arg3);
  func_801D95C4(arg0 + 2, arg1 + 2, arg2 - 5, arg3 - 5, arg4);
}
