#include "bof3/bof3.h"

/* @source 0x80146865 @kind data */
extern u8 D_80146865;
/* @source 0x80146866 @kind data */
extern u8 D_80146866;
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FCC28
 * @behavior Shared-state reset callback of this overlay's record-callback run:
 * it runs the shared front-end startup helper func_8015C088 (0x8015C088), then
 * clears the three shared main-RAM state bytes 0x80146865, 0x80146866 and
 * 0x80146867, clears the shared sub-state byte D_80146875 and requests primary
 * scene state 0x10 in the shared primary state byte D_80146874. It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across the
 * call. The constant 0x10 is materialised once in $v0 (`addiu $v0,$zero,0x10`
 * immediately after the call) to feed the last `sb`, and each of the five byte
 * stores materialises the 0x8014 page in $at on its own, so the store order
 * 0x80146865, 0x80146866, 0x80146867, D_80146875, D_80146874 is the source
 * statement order. The record-callback dispatcher
 * callCallbackTable (0x801FCA58) reaches the
 * address only through word 5 of the nine-word overlay callback run at
 * 0x801FD644 (0x801FD658 holds 0x801FCC28), so the record byte at offset 0x7A
 * selects it with the value 5; no in-image `jal` targets the address and that
 * table word is its only image reference. Its 76 bytes were the same
 * helper-and-clear shape as the exact siblings
 * clearSharedStateBytesThenRequestPrimaryState8ScenarioScena1200_801FC738
 * (0x801FC738) and
 * clearSharedStateBytesThenRequestPrimaryState0EScenarioScena1100_801FA988
 * (0x801FA988), which run the same helper, clear the same four bytes and differ
 * in no word but the one stored immediate; the
 * neighbouring words 6 and 7 of the same table, 0x801FCC74 (stores 0x11) and
 * 0x801FCCC0 (stores 0x12), carried that shape as well and were still on their
 * asm rows when this seed was written. The name records that proven behavior
 * and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClearSharedRequest16(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0x10;
}
