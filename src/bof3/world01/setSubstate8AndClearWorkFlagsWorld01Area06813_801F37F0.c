#include "bof3/bof3.h"

extern u8 D_80146875;

void func_80196070(void);

/* @source 0x801F37F0
 * @behavior Overlay step: writes 8 into the shared secondary sub-state byte
 * D_80146875 and then runs the shared func_80196070 work-flag reset. Takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across
 * that call and the constant reaches the store through $v0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate8AndClearWorkFlagsWorld01Area06813_801F37F0(void) {
  D_80146875 = 8;
  func_80196070();
}
