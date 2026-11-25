#include "bof3/bof3.h"

void func_800AC934(s32 arg0);
/* @source 0x800C1988
 * @behavior Thin overlay wrapper: calls the helper at 0x800AC934 with argument zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callOwnerRecordHead(void) {
  func_800AC934(0);
}
