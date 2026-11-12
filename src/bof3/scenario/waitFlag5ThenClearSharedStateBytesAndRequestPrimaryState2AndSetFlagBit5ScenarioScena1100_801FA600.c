#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FA600
 * @behavior Overlay per-record callback: it first polls the shared flag word
 * D_8014686C (0x8014686C) through the two-argument flag helper
 * func_8015B5D4 with id 5, and a non-zero return ends the handler there, so
 * the bnez arm jumps straight to the epilogue with no state touched. On a zero
 * return it runs the shared front-end startup helper func_8015C088
 * (0x8015C088), re-reads D_8014686C after that call, clears the three shared
 * main-RAM state bytes 0x80146865, 0x80146866 and 0x80146867, clears the
 * shared secondary sub-state byte D_80146875 and requests primary scene state 2
 * by storing 2 in the shared primary state byte D_80146874, and finally applies
 * the paired flag helper func_8015B580 to the same word with id 5. It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across the
 * three calls, and the id 5 of both flag-helper calls is materialised in the
 * jal delay slot as `addiu $a1,$zero,0x5`. The constant 2 is materialised once
 * in $v0 (`addiu $v0,$zero,0x2` immediately after the startup call) to feed the
 * last byte store, and each of the five byte stores materialises the 0x8014
 * page in $at on its own. The dispatcher dispatchRecordCallbackScenarioScena1100_801FA5B8
 * invokes it indirectly through word 1 of the 29-entry overlay callback run at
 * 0x801FAF24 (0x801FAF28 holds 0x801FA600), so the record byte at offset 0x7A
 * selects it with the value 1; neither a jal nor a j in the image reaches the
 * address, so its argument roles stay structural. A payload scan over every
 * out/binaries entry found this 116-byte span at offset 0x3A00 only, so no
 * shipped payload holds a byte-identical copy. Its body is the clear-and-request
 * shape of the exact immediate neighbour
 * clearSharedStateBytesThenRequestPrimaryState3ScenarioScena1100_801FA674
 * (0x801FA674) behind the flag-5 gate of the exact
 * waitFlag5ThenToggleModeFlags36AndRequestPrimaryState4Substate7ScenarioScena0200_801FD1DC
 * (0x801FD1DC); the address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitFlag5ThenClearSharedStateBytesAndRequestPrimaryState2AndSetFlagBit5ScenarioScena1100_801FA600(void) {
  if (func_8015B5D4(D_8014686C, 5) == 0) {
    func_8015C088();
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146867 = 0;
    D_80146875 = 0;
    D_80146874 = 2;
    func_8015B580(D_8014686C, 5);
  }
}
