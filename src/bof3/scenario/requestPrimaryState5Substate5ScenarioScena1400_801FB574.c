#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FB574
 * @behavior Runs the shared front-end startup helper func_8015C088
 * (0x8015C088) and then requests primary scene state 5 with secondary
 * sub-state 5 by storing 5 in the shared primary state byte D_80146874 and 5
 * in the shared secondary sub-state byte D_80146875. It takes no arguments and
 * returns nothing, and its 0x18-byte frame only keeps $ra across the call; the
 * constant 5 is materialised once in $v0 and reused by both byte stores, each
 * of which materialises the 0x8014 page in $at on its own. No in-image jal
 * targets the address; its only image word is 0x801FC47C, entry 5 of the
 * twenty-two-word pointer run at 0x801FC468 (0x801FC468 through 0x801FC4BC)
 * that func_801FB37C (0x801FB37C) indexes with the byte at offset 0x7A of its
 * first argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState5Substate5ScenarioScena1400_801FB574(void) {
  func_8015C088();
  D_80146874 = 5;
  D_80146875 = 5;
}
