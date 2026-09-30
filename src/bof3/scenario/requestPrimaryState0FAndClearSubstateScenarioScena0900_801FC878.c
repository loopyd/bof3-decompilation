#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC878
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 0x0F by storing 0x0F in the main-RAM state byte
 * D_80146874 and clearing the secondary sub-state byte D_80146875. Takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across the
 * call, and each of the two byte stores materialises the 0x8014 page in $at on
 * its own, which is how the psyq compiler emits separate shared-byte stores. No
 * in-image jal targets the address: its only image word is 0x801FE468, entry 13
 * of the 16-word handler table at 0x801FE434 that the overlay dispatcher
 * func_801FC5E4 indexes with byte 0x7A of a record (passing the shared scenario
 * flag word D_8014686C in the second argument register), so the overlay reaches
 * this handler only indirectly. Its 0x34 bytes are the same shape as the exact
 * sibling requestPrimaryState0AAndClearSubstateScenarioScena0900_801FC6DC
 * (0x801FC6DC), which differs only in the state constant it stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState0FAndClearSubstateScenarioScena0900_801FC878(void) {
  func_8015C088();
  D_80146874 = 0xF;
  D_80146875 = 0;
}
