#include "bof3/bof3.h"

u8 func_801665A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F5BAC
 * @behavior Overlay wrapper: asks the same-binary helper at 0x801665A0 for one
 *           unit of the shared countdown entry whose identifier is 71 (0x47) in
 *           slot 1, passing (1, 71, 1, 0) and discarding the helper's status
 *           byte; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownId71InSlot1World02Area07714_801F5BAC(void) {
  func_801665A0(1, 71, 1, 0);
}
