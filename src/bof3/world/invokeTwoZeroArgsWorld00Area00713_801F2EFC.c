#include "bof3/bof3.h"

void func_8015D404(u32 arg0, s32 arg1);

/* @source 0x801F2EFC
 * @behavior Thin overlay wrapper: calls the shared selection-effect helper at
 * 0x8015D404 with both arguments zero (no effect index, no value), then
 * returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoZeroArgsWorld00Area00713_801F2EFC(void) {
  func_8015D404(0, 0);
}
