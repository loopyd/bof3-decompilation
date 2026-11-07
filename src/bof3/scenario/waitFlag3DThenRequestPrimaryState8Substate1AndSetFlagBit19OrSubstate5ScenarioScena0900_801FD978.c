#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FD978
 * @behavior Overlay handler reached only indirectly: the address is entry 7
 * (the word stored at 0x801FE4D8) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 14) and calls with jalr, and the lookup helper can only select
 * indices 0 through 13 for that call. It first takes the two-stage wait
 * func_8015B5D4 on the shared flag word D_8014686C (0x8014686C, read with
 * lui/lw into $a0) for wait id 0x3D. A non-zero return runs the shared
 * front-end startup helper func_8015C088 and then requests primary scene state
 * 8 with secondary sub-state 1 by storing 1 in the shared sub-state byte
 * D_80146875 and then 8 in the shared state byte D_80146874, arms scenario flag
 * bit 0x19 of the shared flag word through func_8015B580(D_8014686C, 0x19) and
 * answers 1, that constant filling the j delay slot of the jump to the shared
 * epilogue while the reloaded flag word sits ahead of both byte stores. A zero
 * return runs the same startup helper and requests the same primary state 8
 * with sub-state 5 instead: it stores 5 in D_80146875 and then 8 in
 * D_80146874, answers 1 with the constant materialised ahead of the two stores
 * and falls into the shared epilogue without reading or writing the flag word.
 * It takes no arguments, reads no other state and writes no other byte, and its
 * 0x18-byte frame only keeps $ra across the calls; each byte store
 * materialises the 0x8014 page in $at on its own. Its 140 bytes differ from the
 * preceding table word
 * waitFlag3DThenRequestPrimaryState8Substate0AndSetFlagBit18OrSubstate5ScenarioScena0900_801FD8F0
 * (0x801FD8F0) only in the stored sub-state constant (0 there against 1 here)
 * and the armed flag bit (0x18 there against 0x19 here), which is shape
 * evidence only: no shipped overlay payload holds a byte-identical copy of this
 * span. The extra 0x4 byte against that 136-byte twin is the materialised 1
 * that the twin's zero store does without.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlag3DThenRequestPrimaryState8Substate1AndSetFlagBit19OrSubstate5ScenarioScena0900_801FD978(void) {
  if (func_8015B5D4(D_8014686C, 0x3D) == 0) {
    func_8015C088();
    D_80146875 = 5;
    D_80146874 = 8;
    return 1;
  }

  func_8015C088();
  D_80146875 = 1;
  D_80146874 = 8;
  func_8015B580(D_8014686C, 0x19);
  return 1;
}
