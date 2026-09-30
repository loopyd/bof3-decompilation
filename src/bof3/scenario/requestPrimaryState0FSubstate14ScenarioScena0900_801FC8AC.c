#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC8AC
 * @behavior Entry 14 of the 16-word in-image callback table at 0x801FE434
 * (its word sits at 0x801FE46C) that the overlay dispatcher func_801FC5E4
 * indexes with byte 0x7A of a record and hands the shared word D_8014686C in
 * the second argument register, so the overlay reaches this handler only
 * indirectly. It runs the shared front-end startup helper func_8015C088 and
 * then requests primary scene state 0x0F with secondary sub-state 0x14 by
 * storing 0x0F in the shared state byte D_80146874 and then 0x14 in the shared
 * sub-state byte D_80146875. Takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call, and each of the two byte
 * stores materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. Its 0x38 bytes are the same shape
 * as the exact sibling requestPrimaryState8Substate0FScenarioScena0900_801FC838
 * (0x801FC838), which differs only in the two stored constants.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState0FSubstate14ScenarioScena0900_801FC8AC(void) {
  func_8015C088();
  D_80146874 = 0xF;
  D_80146875 = 0x14;
}
