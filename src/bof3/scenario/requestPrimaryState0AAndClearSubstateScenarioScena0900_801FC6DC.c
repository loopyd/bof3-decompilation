#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC6DC
 * @behavior Entry 4 of the 16-entry in-image callback table at 0x801FE434
 * (word at 0x801FE444) that the overlay dispatcher func_801FC5E4 indexes with
 * byte 0x7A of a record and hands the shared word D_8014686C, so the overlay
 * reaches this handler only indirectly. It runs the shared front-end startup
 * helper func_8015C088 and then requests primary scene state 0x0A in the shared
 * state byte D_80146874 while clearing the secondary sub-state byte D_80146875.
 * Takes no arguments and returns nothing; the 0x18-byte frame only keeps $ra
 * across the call and each of the two byte stores materialises the 0x8014 page
 * in $at on its own, which is how the psyq compiler emits separate shared-byte
 * stores. Its 0x34 bytes are the same shape as the exact sibling
 * requestPrimaryState17AndClearSubstateScenarioScena0200_801FD574
 * (0x801FD574), which runs the same helper and clears D_80146875 before
 * storing its own primary state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState0AAndClearSubstateScenarioScena0900_801FC6DC(void) {
  func_8015C088();
  D_80146874 = 0xA;
  D_80146875 = 0;
}
