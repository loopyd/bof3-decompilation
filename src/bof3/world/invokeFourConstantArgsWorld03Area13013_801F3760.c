#include "bof3/bof3.h"

/* @source 0x8015D300 @kind function */
extern void func_8015D300(u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* @source 0x801F3760
 * @behavior Calls the shared executable helper at 0x8015D300 with arguments
 * 0, 1, 100 and 100, then returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeFourConstantArgsWorld03Area13013_801F3760(void) {
  func_8015D300(0u, 1u, 100u, 100u);
}
