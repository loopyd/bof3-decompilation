#include "bof3/bof3.h"

void func_801F34CC(void);
void dispatchStateByte3Table4f74World03Area11513_801F3690(void);

/* @source 0x801F34A4
 * @behavior Thin overlay wrapper: calls the work-record state-byte-2 dispatcher at 0x801F34CC and the state-byte-3 dispatcher at 0x801F3690 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld03Area11513_801F34A4(void) {
  func_801F34CC();
  dispatchStateByte3Table4f74World03Area11513_801F3690();
}
