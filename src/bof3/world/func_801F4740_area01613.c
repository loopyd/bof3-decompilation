#include "bof3/world/area01613_internal.h"

/* @source 0x801F4740
 * @behavior Calls the shared helpers `func_8014D978` and `func_8014D260` with no
 * arguments in that order, ignores both results, and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F4740(void) {
  func_8014D978();
  func_8014D260();
}
