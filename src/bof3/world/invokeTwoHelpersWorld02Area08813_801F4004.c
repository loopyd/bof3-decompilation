#include "bof3/bof3.h"

void func_801F402C(void);
void callStateByte3Table61(void);

/* @source 0x801F4004
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F402C and 0x801F41F0 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld02Area08813_801F4004(void) {
  func_801F402C();
  callStateByte3Table61();
}
