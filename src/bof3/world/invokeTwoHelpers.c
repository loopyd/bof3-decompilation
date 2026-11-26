#include "bof3/bof3.h"

void func_801F34CC(void);
void callStateByte3Table4(void);

/* @source 0x801F34A4
 * @behavior Thin overlay wrapper: calls the work-record state-byte-2 dispatcher at 0x801F34CC and the state-byte-3 dispatcher at 0x801F3690 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpers(void) {
  func_801F34CC();
  callStateByte3Table4();
}
