#include "bof3/bof3.h"

void func_801F3500(void);
void callStateByte3Table52(void);

/* @source 0x801F34D8
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F3500 and 0x801F36C4 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeStateHelperPair(void) {
  func_801F3500();
  callStateByte3Table52();
}
