#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC6A4
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 4 with secondary sub-state 5: it stores 5 in the
 * shared sub-state byte D_80146875 and then 4 in the shared state byte
 * D_80146874. Takes no arguments and returns nothing; the 0x18-byte frame only
 * keeps $ra across the call, and each of the two byte stores materialises the
 * 0x8014 page in $at on its own, which is how the psyq compiler emits separate
 * shared-byte stores. No in-image jal targets the address: its only image word
 * is 0x801FE440, entry 3 of the 16-word handler table at 0x801FE434 that the
 * overlay dispatcher func_801FC5E4 indexes with byte 0x7A of a record (passing
 * the shared scenario flag word D_8014686C in the second argument register), so
 * the overlay reaches this handler only indirectly. Its 0x38 bytes keep the
 * sub-state-then-state store order of the exact sibling
 * requestPrimaryState1Substate10ScenarioScena1200_801FC7D4 (0x801FC7D4), and
 * differ from it only in the two stored constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState4Substate5ScenarioScena0900_801FC6A4(void) {
  func_8015C088();
  D_80146875 = 5;
  D_80146874 = 4;
}
