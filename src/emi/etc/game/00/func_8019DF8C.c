#include "internal.h"

/* @behavior zeroes the u16 at D_8014932C, then clears work area flags 0x00-0x04.
 * @source 0x8019DF8C
 */
void func_8019DF8C(void) {
  D_8014932C = 0;
  clearWorkFlags();
}
