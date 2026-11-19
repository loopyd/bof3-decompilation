#include "bof3/bof3.h"

/* @source 0x80146864 @kind data */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind data */
extern u8 D_80146865;
/* @source 0x80146866 @kind data */
extern u8 D_80146866;
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* @source 0x8014686C @kind data */
extern u32 D_8014686C;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;

void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FCB1C
 * @behavior Record-callback entry 2 of this overlay's nine-word callback run:
 * it sets flag bit 0xB of the shared scenario flag word D_8014686C through the
 * shared bit-set helper func_8015B580 (0x8015B580), which receives that word
 * loaded with `lui`/`lw` and the bit index 0xB, then runs the shared front-end
 * startup helper func_8015C088 (0x8015C088), then clears the shared progress
 * byte g_ScenarioProgress (0x80146864), the three shared main-RAM state bytes
 * 0x80146865, 0x80146866 and 0x80146867 and the shared sub-state byte
 * D_80146875, and requests primary scene state 8 in the shared primary state
 * byte D_80146874. It takes no arguments and returns nothing; the 0x18-byte
 * frame only keeps $ra across the two calls, the constant 8 is materialised
 * once in $v0 immediately after the second call to feed the last `sb`, and
 * each of the six byte stores materialises the 0x8014 page in $at on its own,
 * so the store order 0x80146864, 0x80146865, 0x80146866, 0x80146867,
 * D_80146875, D_80146874 is the source statement order. The record-callback
 * dispatcher dispatchRecordCallbackScenarioScena0500_801FCA58 (0x801FCA58)
 * reaches the address only through word 2 of the nine-word overlay callback
 * run at 0x801FD644 (0x801FD64C holds 0x801FCB1C), so the record byte at offset
 * 0x7A selects it with the value 2; no in-image `jal` targets the address and
 * that table word is its only image reference. Its 100 bytes are the body of
 * the exact in-target sibling
 * clearProgressAndSharedStateBytesThenRequestPrimaryState0DScenarioScena0500_801FCBD4
 * (0x801FCBD4) preceded by that flag-bit call. The neighbouring word 1
 * callback func_801FCAA0 differs by guarding the whole body on the shared flag
 * test func_8015B5D4 (0x8015B5D4) and by its own bit index 2 and stored
 * 0x15, and word 3's
 * clearProgressAndSharedStateBytesThenRequestPrimaryState0CScenarioScena0500_801FCB80
 * (0x801FCB80) omits the flag-bit call and stores 0xC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScenarioFlagBit0BThenClearProgressAndSharedStateBytesAndRequestPrimaryState8ScenarioScena0500_801FCB1C(void) {
  func_8015B580(D_8014686C, 0xB);
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 8;
}
