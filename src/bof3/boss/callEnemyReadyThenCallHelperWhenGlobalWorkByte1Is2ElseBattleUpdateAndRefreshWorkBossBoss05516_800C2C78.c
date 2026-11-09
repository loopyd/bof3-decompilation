#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, published outside this
 * overlay's image. This target's generated undefined_syms_auto.txt already
 * binds the address because the remaining assembly rows of this payload load
 * it, and the sibling battle lifts
 * callHelperWhenGlobalWorkByte1Is2ElseCallEnemyReadyBattleUpdateAndRefreshWork
 * BossBoss05516_800C2FA4 and
 * incrementWorkByte2WhenGlobalWorkByte1Is2BmagicMagic15103_801EFD80 test the
 * same record byte +1 as the global work byte. Every access here is
 * byte-addressed, so the declaration keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helpers of this overlay step, called without
 * arguments here. The readiness helper 0x801E30F8 is enemyReadyOrHelper1 of
 * emi/battle/battle/03 in the main executable, whose result this body
 * discards; the roles of 0x801E5988 and 0x8014D4E0 are not established, so
 * both keep their address-anchored spelling. This target's generated
 * undefined_funcs_auto.txt binds all three. */
extern u8 func_801E30F8(void); /* @source 0x801E30F8 */
extern void func_801E5988(void); /* @source 0x801E5988 */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 */

/* This overlay's own work-refresh step, defined in its own source file. */
extern void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void); /* @source 0x800C3284 */

/* @source 0x800C2C78
 * @behavior Overlay step body, entry 1 of the two-entry handler pair that
 * dispatchStateByte2LocalPairBossBoss05516_800C2B48 seeds with 0x800C2BA0 then
 * this function: it calls the EXE-side readiness helper 0x801E30F8 first and
 * discards the result, then reads the shared battle work record pointer at
 * 0x801EB4E0 and, when that record unsigned byte +1 equals 2, calls the shared
 * helper 0x801E5988 alone; otherwise it calls the shared battle update
 * 0x8014D4E0 and this overlay own work refresh 0x800C3284, in that order and
 * discarding each result. It takes no arguments and writes no state of its
 * own: the 0x18-byte frame holds nothing but $ra at 0x10 across the calls and
 * the two arms join at a single epilogue rather than tail calling. The
 * readiness call is unconditional here, unlike the exact sibling
 * callHelperWhenGlobalWorkByte1Is2ElseCallEnemyReadyBattleUpdateAndRefreshWork
 * BossBoss05516_800C2FA4 where it sits in the else arm and the record byte is
 * read before the frame adjustment, so this body is not that one and the
 * statement order is taken from these bytes. The guard materializes 2 and
 * compares the loaded byte against it, so the test is an equality on a re-read
 * cell. No payload data word holds 0x800C2C78; the dispatcher above
 * materializes the address with lui plus addiu into its frame, so this entry is
 * reached through that pair and has no jal caller. The roles of work-record
 * byte +1 beyond its role as the global work byte, and of the three EXE-side
 * helpers, are not established, so they stay offset- and address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEnemyReadyThenCallHelperWhenGlobalWorkByte1Is2ElseBattleUpdateAndRefreshWorkBossBoss05516_800C2C78(void) {
  func_801E30F8();
  if (D_801EB4E0[1] == 2) {
    func_801E5988();
  } else {
    func_8014D4E0();
    refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
  }
}
