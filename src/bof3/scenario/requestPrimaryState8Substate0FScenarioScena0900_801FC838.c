#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC838
 * @behavior Entry 11 of the 16-word in-image callback table at 0x801FE434
 * (its word sits at 0x801FE460) that the overlay dispatcher func_801FC5E4
 * indexes with byte 0x7A of a record and hands the shared word D_8014686C in
 * the second argument register, so the overlay reaches this handler only
 * indirectly. It runs the shared front-end startup helper func_8015C088 and
 * then requests primary scene state 8 with secondary sub-state 0x0F by storing
 * 8 in the shared state byte D_80146874 and then 0x0F in the shared sub-state
 * byte D_80146875. Takes no arguments and returns nothing; the 0x18-byte frame
 * only keeps $ra across the call, and each of the two byte stores materialises
 * the 0x8014 page in $at on its own, which is how the psyq compiler emits
 * separate shared-byte stores. Its 0x38 bytes keep the state-then-sub-state
 * store order of the exact sibling
 * requestPrimaryState0ASubstate1ScenarioScena0900_801FC710 (0x801FC710), which
 * differs only in the two stored constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState8Substate0FScenarioScena0900_801FC838(void) {
  func_8015C088();
  D_80146874 = 8;
  D_80146875 = 0xF;
}
