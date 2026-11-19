#include "bof3/bof3.h"

/* @source 0x80146864 @kind data */
extern u8 g_ScenarioProgress;
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

/* @source 0x801FCB80
 * @behavior Record-callback entry 3 of this overlay's nine-word callback run:
 * it runs the shared front-end startup helper func_8015C088 (0x8015C088),
 * then clears the shared progress byte g_ScenarioProgress (0x80146864),
 * clears the three shared main-RAM state bytes 0x80146865, 0x80146866 and
 * 0x80146867, clears the shared sub-state byte D_80146875 and requests
 * primary scene state 0xC in the shared primary state byte D_80146874. It
 * takes no arguments and returns nothing; the 0x18-byte frame only keeps $ra
 * across the call. The constant 0xC is materialised once in $v0 (`addiu
 * $v0,$zero,0xC` immediately after the call) to feed the last `sb`, and each
 * of the six byte stores materialises the 0x8014 page in $at on its own, so
 * the store order 0x80146864, 0x80146865, 0x80146866, 0x80146867,
 * D_80146875, D_80146874 is the source statement order. The record-callback
 * dispatcher dispatchRecordCallbackScenarioScena0500_801FCA58 (0x801FCA58)
 * reaches the address only through word 3 of the nine-word overlay callback
 * run at 0x801FD644 (0x801FD650 holds 0x801FCB80), so the record byte at
 * offset 0x7A selects it with the value 3; no in-image `jal` targets the
 * address and that table word is its only image reference. Its 84 bytes at
 * 0x801FCB80..0x801FCBD3 compare equal to the exact in-target sibling
 * clearProgressAndSharedStateBytesThenRequestPrimaryState0DScenarioScena0500_801FCBD4
 * (0x801FCBD4) in every byte except the one stored primary-state immediate
 * inside the `addiu $v0,$zero,imm` word, so that sibling supplied the
 * statement order, operand forms and name this body writes and the immediate
 * 0xC is read from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState0CScenarioScena0500_801FCB80(void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0xC;
}
