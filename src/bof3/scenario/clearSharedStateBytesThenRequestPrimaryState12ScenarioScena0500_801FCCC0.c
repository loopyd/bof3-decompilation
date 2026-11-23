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

/* @source 0x801FCCC0
 * @behavior Shared-state reset callback of this overlay's record-callback run:
 * it runs the shared front-end startup helper func_8015C088 (0x8015C088), then
 * clears the three shared main-RAM state bytes 0x80146865, 0x80146866 and
 * 0x80146867, clears the shared sub-state byte D_80146875 and requests primary
 * scene state 0x12 in the shared primary state byte D_80146874. It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across the
 * call. The constant 0x12 is materialised once in $v0 (`addiu $v0,$zero,0x12`
 * immediately after the call) to feed the last `sb`, and each of the five byte
 * stores materialises the 0x8014 page in $at on its own, so the store order
 * 0x80146865, 0x80146866, 0x80146867, D_80146875, D_80146874 is the source
 * statement order. The record-callback dispatcher
 * callCallbackTable (0x801FCA58) reaches the
 * address only through word 7 of the nine-word overlay callback run at
 * 0x801FD644 (0x801FD660 holds 0x801FCCC0), so the record byte at offset 0x7A
 * selects it with the value 7; no in-image `jal` targets the address and that
 * table word is its only image reference. Its 76 bytes were seeded from the
 * exact sibling clearSharedStateBytesThenRequestPrimaryState11ScenarioScena0500_801FCC74
 * (0x801FCC74), which runs the same helper, clears the same four bytes and
 * differs in no word but the one stored immediate: a 76-byte compare of the two
 * payload spans under the target binary is unequal only at span offset 0x10,
 * where this span holds 0x24020012 and the sibling holds 0x24020011. The
 * neighbouring word 5 sibling stateClearSharedRequest16
 * (0x801FCC28, stores 0x10) also carried that shape. The name records that
 * proven behavior and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSharedStateBytesThenRequestPrimaryState12ScenarioScena0500_801FCCC0(void) {
  func_8015C088();
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0x12;
}
