#include "bof3/ui/shop00_internal.h"

/* @source 0x801D910C
 * @behavior shop icon wrapper: when the kind byte arg2 equals 4 and the signed
 *           main-RAM byte D_80146870 is at least 8, substitutes the kind 0xB;
 *           then forwards the two zero-extended u16 coordinates, the kind byte
 *           and the mode byte arg3 to the shop icon emitter func_801D8FC0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D910C(u16 arg0, u16 arg1, u8 arg2, u8 arg3) {
  if (arg2 == 4 && D_80146870 >= 8) {
    arg2 = 0xB;
  }
  func_801D8FC0(arg0, arg1, arg2, arg3);
}
