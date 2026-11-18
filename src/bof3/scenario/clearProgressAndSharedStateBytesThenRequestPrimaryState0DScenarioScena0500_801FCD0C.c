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

/* @source 0x801FCD0C
 * @behavior Shared-state reset handler at the last word of this overlay's
 * two handler runs: it runs the shared front-end startup helper func_8015C088
 * (0x8015C088), then clears the shared progress byte g_ScenarioProgress
 * (0x80146864), clears the three shared main-RAM state bytes 0x80146865,
 * 0x80146866 and 0x80146867, clears the shared sub-state byte D_80146875 and
 * requests primary scene state 0xD in the shared primary state byte
 * D_80146874. It takes no arguments and returns nothing; the 0x18-byte frame
 * only keeps $ra across the call. The constant 0xD is materialised once in $v0
 * (`addiu $v0,$zero,0xD` immediately after the call) to feed the last `sb`, and
 * each of the six byte stores materialises the 0x8014 page in $at on its own,
 * so the store order 0x80146864, 0x80146865, 0x80146866, 0x80146867,
 * D_80146875, D_80146874 is the source statement order. The address has one
 * image word, 0x801FD664, and no in-image `jal`: that word is word 31 (the last)
 * of the thirty-two-word overlay primary handler table
 * D_801FD5E8 (0x801FD5E8) that
 * dispatchPrimaryStateHandlerScenarioScena0500_801F83EC (0x801F83EC) indexes
 * with D_80146874, and at the same time word 8 (the last) of the nine-word
 * record-callback run at 0x801FD644 that
 * dispatchRecordCallbackScenarioScena0500_801FCA58 (0x801FCA58) indexes with the
 * record byte at offset 0x7A, so the two dispatchers select this entry with
 * primary state 31 and record byte 8. Its 84 bytes at 0x801FCD0C..0x801FCD5F
 * compare equal word for word with the exact in-target sibling
 * clearProgressAndSharedStateBytesThenRequestPrimaryState0DScenarioScena0500_801FCBD4
 * (0x801FCBD4), so that sibling supplied the statement order, operand forms and
 * name this body writes, and every literal here is that same span's own
 * immediate. The name records the proven behavior and the address anchor keeps
 * it distinct from that sibling's identical body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState0DScenarioScena0500_801FCD0C(void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0xD;
}
