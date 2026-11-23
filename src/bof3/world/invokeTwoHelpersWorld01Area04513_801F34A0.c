#include "bof3/bof3.h"

void func_801F34C8(void);
void callStateByte3Table5364(void);

/* @source 0x801F34A0
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F34C8 and 0x801F368C in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld01Area04513_801F34A0(void) {
  func_801F34C8();
  callStateByte3Table5364();
}
