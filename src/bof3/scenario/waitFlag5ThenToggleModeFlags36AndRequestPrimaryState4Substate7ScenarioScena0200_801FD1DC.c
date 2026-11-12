#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u16 D_80146258;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FD1DC
 * @behavior Overlay record callback reached from the local callback table
 * beside dispatchRecordCallbackByByte7AScenarioScena0200_801FD084; it reads
 * neither argument. It first takes the two-stage wait func_8015B5D4 on the
 * shared scenario flag word D_8014686C (0x8014686C, read with lui/lw into
 * $a0) for wait id 5: a non-zero return leaves the bnez arm jumping straight
 * to the shared epilogue, so the handler ends without touching any state. A
 * zero return means the wait is done, so it flips bits 0x36 of the shared
 * halfword mode word D_80146258 with lhu/xori/sh, runs the shared front-end
 * startup helper func_8015C088, sets the shared sub-state byte D_80146875 to
 * 7, clears the shared bytes 0x80146865 and 0x80146866, and requests primary
 * scene state 4 through D_80146874. The 0x18-byte frame only keeps $ra across
 * the two calls, the wait's constant argument is materialised in the jal
 * delay slot, and each byte store materialises the 0x8014 page in $at on its
 * own. The role of the flipped bits and of the cleared bytes remains unknown.
 * Its 120 bytes are the reviewed exact neighbour
 * waitFlag5ThenToggleModeFlags36AndRequestPrimaryState4Substate5ScenarioScena0200_801FD164
 * (0x801FD164) shape, which differs only in the sub-state constant, and the
 * store tail repeats clearSharedBytes6566AndRequestPrimaryState0BSubstate7ScenarioScena0200_801FD0CC
 * (0x801FD0CC); a payload scan over every out/binaries entry found this 120-byte
 * span at offset 0x65DC only, so no shipped payload holds a byte-identical
 * copy and the neighbour supplied shape alone. The address anchor keeps the
 * name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitFlag5ThenToggleModeFlags36AndRequestPrimaryState4Substate7ScenarioScena0200_801FD1DC(void) {
  if (func_8015B5D4(D_8014686C, 5) == 0) {
    D_80146258 ^= 0x36;
    func_8015C088();
    D_80146875 = 7;
    D_80146865 = 0;
    D_80146866 = 0;
    D_80146874 = 4;
  }
}
