#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, published outside this
 * overlay's image. This target's generated undefined_syms_auto.txt already
 * binds the address because the remaining assembly rows of this payload load
 * it, and the sibling battle lifts
 * incrementWorkByte2WhenGlobalWorkByte1Is2BmagicMagic15103_801EFD80 and
 * decrementWorkByte9WhenGlobalWorkByte1Is3ThenAdvanceWorkByte1WhenZeroBmagic
 * Magic09503_801F0070 test the same record byte +1 as the global work byte.
 * Every access here is byte-addressed, so the declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helpers of this overlay step, called without
 * arguments here. The readiness helper 0x801E30F8 is enemyReadyOrHelper1 of
 * emi/battle/battle/03 in the main executable, whose result this body
 * discards; the roles of 0x801E5988 and 0x8014D4E0 are not established, so
 * both keep their address-anchored spelling. This target's generated
 * undefined_funcs_auto.txt binds all three. */
extern void func_801E5988(void); /* @source 0x801E5988 */
extern u8 func_801E30F8(void); /* @source 0x801E30F8 */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 */

/* This overlay's own work-refresh step, defined in its own source file. */
extern void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void); /* @source 0x800C3284 */

/* @source 0x800C2FA4
 * @behavior Overlay step body, entry 1 of the two-entry handler pair that
 * dispatchStateByte2LocalPairBossBoss05516_800C2E74 seeds with 0x800C2ECC then
 * this function: it reads the shared battle work record pointer at 0x801EB4E0
 * once, and when that record's unsigned byte +1 equals 2 it calls the shared
 * helper 0x801E5988 alone; otherwise it calls the EXE-side readiness helper
 * 0x801E30F8, the shared battle update 0x8014D4E0 and this overlay's own work
 * refresh 0x800C3284, in that order and discarding each result. It takes no
 * arguments and writes no state of its own: the 0x18-byte frame holds nothing
 * but $ra at 0x10 across the calls, the pointer-cell load precedes the frame
 * adjustment, and the two arms join at a single epilogue rather than tail
 * calling. The guard materializes 2 and compares the loaded byte against it,
 * so the test is an equality on a re-read cell. No payload data word holds
 * 0x800C2FA4; the dispatcher above materializes the address with lui plus
 * addiu into its frame, so this entry is reached through that pair and has no
 * jal caller. The roles of work-record byte +1 beyond its role as the global
 * work byte, and of the three EXE-side helpers, are not established, so they
 * stay offset- and address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callHelperWhenGlobalWorkByte1Is2ElseCallEnemyReadyBattleUpdateAndRefreshWorkBossBoss05516_800C2FA4(void) {
  if (D_801EB4E0[1] == 2) {
    func_801E5988();
  } else {
    func_801E30F8();
    func_8014D4E0();
    refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
  }
}
