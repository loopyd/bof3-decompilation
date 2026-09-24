#include "bof3/bof3.h"

void func_801F34FC(void);
void func_801F36C0(void);

/* @source 0x801F34D4
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F34FC and 0x801F36C0 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld02Area08713_801F34D4(void) {
  func_801F34FC();
  func_801F36C0();
}
