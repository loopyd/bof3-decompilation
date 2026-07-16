#include "internal.h"

/* @behavior initializes one small ui state bundle with fixed halfwords and one
 * caller-provided byte.
 * @source 0x801D9428
 */
void func_801D9428(u8 arg0) {
  volatile u8*  state8;
  volatile u16* state16;

  func_80158DB8(4, 3);
  state8 = (volatile u8*)0x80140000;
  state16 = (volatile u16*)0x80140000;
  state8[0x83c2] = 3;
  state16[0x41e2] = 0x7c;
  state8[0x83c3] = arg0;
  state16[0x41e3] = 0x2a;
}
