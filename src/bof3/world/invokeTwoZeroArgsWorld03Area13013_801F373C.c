#include "bof3/bof3.h"

extern void func_8015D404(u32 arg0, s32 arg1);

/* @source 0x801F373C
 * @behavior Calls the shared executable helper at 0x8015D404 with both arguments
 * zero, then returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoZeroArgsWorld03Area13013_801F373C(void) {
  func_8015D404(0, 0);
}
