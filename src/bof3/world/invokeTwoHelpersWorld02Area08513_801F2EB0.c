#include "bof3/bof3.h"

void func_801F2ED8(void);
void func_801F2FCC(void);

/* @source 0x801F2EB0
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F2ED8 and 0x801F2FCC in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld02Area08513_801F2EB0(void) {
  func_801F2ED8();
  func_801F2FCC();
}
