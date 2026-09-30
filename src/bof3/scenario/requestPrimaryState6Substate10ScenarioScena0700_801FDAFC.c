#include "bof3/bof3.h"

extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FDAFC
 * @behavior Runs the shared front-end startup helper func_8015C088 and then
 * requests primary scene state 6 with secondary sub-state 0x0A by storing 6 in
 * the shared state byte D_80146874 and 0x0A in the shared sub-state byte
 * D_80146875. It takes no arguments and returns nothing, and its 0x18-byte
 * frame only keeps $ra across the call; each of the two byte stores
 * materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. No in-image jal targets the
 * address; its only image word is 0x801FDFA8, the first slot of the five-slot
 * pointer run at 0x801FDFA8 (0x801FDAFC / 0x801FDB34 / 0x801FDB80 /
 * 0x801FDBCC / 0x801FDBF4) that no code in this target reaches directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestPrimaryState6Substate10ScenarioScena0700_801FDAFC(void) {
  func_8015C088();
  D_80146874 = 6;
  D_80146875 = 0xA;
}
