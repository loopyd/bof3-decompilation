#include "bof3/bof3.h"

void func_8014E4BC(void* arg0);

/* @source 0x801F3BA4
 * @behavior Thin overlay wrapper: calls the helper at 0x8014E4BC with the
 *           work record held in scratchpad pointer-slot 0x1F800044 as its
 *           only argument; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWithWorkWorld02Area08513_801F3BA4(void) {
  func_8014E4BC(SPAD_PTR_TABLE(u8)[0x11]);
}
