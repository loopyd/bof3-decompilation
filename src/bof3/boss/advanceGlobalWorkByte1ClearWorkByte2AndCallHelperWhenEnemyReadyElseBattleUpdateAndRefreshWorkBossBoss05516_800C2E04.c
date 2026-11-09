#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, published outside this
 * overlay's image; this target's generated undefined_syms_auto.txt already
 * binds the address. Every access here is byte-addressed, so the declaration
 * keeps the byte view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable helpers of this overlay step, called without
 * arguments here. The readiness helper 0x801E3160 is enemyReadyOrHelper2 of
 * emi/battle/battle/03 and returns an unsigned byte, which this body tests;
 * the roles of 0x801E5988 and 0x8014D4E0 are not established, so both keep
 * their address-anchored spelling. This target's generated
 * undefined_funcs_auto.txt binds all three. */
extern u8 func_801E3160(void); /* @source 0x801E3160 */
extern void func_801E5988(void); /* @source 0x801E5988 */
extern void func_8014D4E0(void); /* @source 0x8014D4E0 */

/* This overlay's own work-refresh step, defined in its own source file. */
extern void refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284(void); /* @source 0x800C3284 */

/* @source 0x800C2E04
 * @behavior Overlay step body, entry 1 of the two-entry handler pair that
 * dispatchStateByte2LocalPairBossBoss05516_800C2CD4 seeds with 0x800C2D2C then
 * this function: it calls the EXE-side readiness helper 0x801E3160 and, when
 * that helper unsigned byte result is nonzero, advances byte +1 of the shared
 * battle work record at 0x801EB4E0, clears that record byte +2 and calls the
 * shared helper 0x801E5988 alone; otherwise it calls the shared battle update
 * 0x8014D4E0 and this overlay own work refresh 0x800C3284, in that order and
 * discarding each result. It takes no arguments and returns nothing: the
 * 0x18-byte frame keeps only $ra across the calls and the two arms join at a
 * single epilogue rather than tail calling. Each record access is its own
 * dereference of the global pointer cell, so the increment and the clear each
 * reload the record pointer, and the clear is the store the original placed in
 * the delay slot of the 0x801E5988 call. The sibling
 * callEnemyReadyThenCallHelperWhenGlobalWorkByte1Is2ElseBattleUpdateAndRefreshWork
 * BossBoss05516_800C2C78 shares this body frame, else arm and 0x801E5988 call,
 * but it tests record byte +1 for equality with 2 after an unconditional call
 * to the other readiness helper 0x801E30F8, so this body takes its own helper,
 * its own test and its own then arm from these bytes. The roles of work-record
 * byte +1 beyond its role as the global work byte, of byte +2, and of the three
 * EXE-side helpers are not established, so they stay offset- and
 * address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceGlobalWorkByte1ClearWorkByte2AndCallHelperWhenEnemyReadyElseBattleUpdateAndRefreshWorkBossBoss05516_800C2E04(void) {
  if (func_801E3160() != 0) {
    D_801EB4E0[1]++;
    D_801EB4E0[2] = 0;
    func_801E5988();
  } else {
    func_8014D4E0();
    refreshWorkFromRecordThenAdvancePositionByModeScaleBossBoss05516_800C3284();
  }
}
