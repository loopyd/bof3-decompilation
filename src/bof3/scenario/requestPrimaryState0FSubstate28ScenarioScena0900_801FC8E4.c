#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FC8E4
 * @behavior Entry 15 of the 16-word in-image callback table at 0x801FE434 (its
 * word sits at 0x801FE470) that the overlay dispatcher func_801FC5E4 indexes
 * with unsigned byte 0x7A of a record, handing that record in $a0 and the
 * shared word D_8014686C in $a1, so the overlay reaches this handler only
 * indirectly. It runs the shared front-end startup helper func_8015C088 and
 * then requests primary scene state 0x0F with secondary sub-state 0x28 by
 * storing 0x0F in the shared state byte D_80146874 and then 0x28 in the shared
 * sub-state byte D_80146875. Takes no arguments and returns nothing; the
 * 0x18-byte frame only keeps $ra across the call, and each of the two byte
 * stores materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. Its 0x38 bytes are
 * requestPrimaryState0FSubstate14ScenarioScena0900_801FC8AC (0x801FC8AC) with
 * exactly one byte changed, the immediate of the second store (0x14 there).
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState0FSubstate28ScenarioScena0900_801FC8E4(void) {
  func_8015C088();
  D_80146874 = 0xF;
  D_80146875 = 0x28;
}
