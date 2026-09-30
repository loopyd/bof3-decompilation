#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD6BC
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 0x1B with secondary sub-state 5: it stores 5 in
 * the shared sub-state byte D_80146875 and then 0x1B in the shared state byte
 * D_80146874. Takes no arguments and returns nothing; its 0x18-byte frame only
 * keeps $ra across the call, and each of the two byte stores materialises the
 * 0x8014 page in $at on its own, which is how the psyq compiler emits separate
 * shared-byte stores. Its 0x38 bytes repeat the exact helper-then-sub-state-then-
 * state store shape of the sibling lifts of this overlay
 * requestPrimaryState1BSubstate10ScenarioScena0200_801FD53C (0x801FD53C) and
 * requestPrimaryState1ASubstate19ScenarioScena0200_801FD684 (0x801FD684),
 * differing only in the two stored constants. No in-image jal targets the
 * address; its only image word is 0x801FE36C, word 26 of the overlay callback
 * table at 0x801FE304 that func_801FD084 (0x801FD084) indexes with byte 0x7A of
 * its record argument, so the overlay reaches this handler only indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState1BSubstate5ScenarioScena0200_801FD6BC(void) {
  func_8015C088();
  D_80146875 = 5;
  D_80146874 = 0x1B;
}
