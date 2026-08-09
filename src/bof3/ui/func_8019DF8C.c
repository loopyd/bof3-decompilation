#include "bof3/ui/game00_internal.h"

/* @behavior zeroes the u16 at D_8014932C, then clears work area flags 0x00-0x04.
 * @source 0x8019DF8C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8019DF8C(void) {
  D_8014932C = 0;
  clearWorkFlags();
}
