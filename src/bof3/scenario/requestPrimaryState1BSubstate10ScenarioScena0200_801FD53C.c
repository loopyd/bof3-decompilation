#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD53C
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 0x1B with secondary sub-state 0x0A: it stores
 * 0x0A in the shared sub-state byte D_80146875 and then 0x1B in the shared state
 * byte D_80146874. Takes no arguments and returns nothing; its 0x18-byte frame
 * only keeps $ra across the call, and each of the two byte stores materialises
 * the 0x8014 page in $at on its own, which is how the psyq compiler emits
 * separate shared-byte stores. No in-image jal targets the address; its only
 * image word is 0x801FE354, entry 20 of the overlay callback table D_801FE304
 * that func_801FD084 indexes with a record's byte field 0x7A, so the overlay
 * reaches this handler only indirectly. Its 0x38 bytes repeat the exact
 * sub-state-then-state store shape of the sibling lifts
 * requestPrimaryState1Substate10ScenarioScena1200_801FC7D4 (0x801FC7D4,
 * scena12/00) and
 * requestPrimaryState4Substate5ScenarioScena0900_801FC6A4 (0x801FC6A4,
 * scena09/00), differing only in the two stored constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState1BSubstate10ScenarioScena0200_801FD53C(void) {
  func_8015C088();
  D_80146875 = 0xA;
  D_80146874 = 0x1B;
}
