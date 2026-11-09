#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void func_800C2D2C(void);
void advanceGlobalWorkByte1ClearWorkByte2AndCallHelperWhenEnemyReadyElseBattleUpdateAndRefreshWorkBossBoss05516_800C2E04(void);

/* @source 0x800C2CD4
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C2D2C then 0x800C2E04 and calls the entry selected by
 * the record unsigned byte +2 without arguments. The original reads the
 * work-record pointer once ahead of both frame-slot stores, leaves the index
 * unbounded and writes no state of its own: the 0x20-byte frame only keeps $ra
 * across the indirect call. Its body is the same 22 instructions and 0x58 bytes
 * as the exact siblings 0x800C2728, 0x800C2888, 0x800C29E8 and 0x800C2B48 and
 * differs from them only in the pair it seeds. The word at 0x800C3608 holds
 * this function address and no other word of the payload does, so it is entry 4
 * of the 0x800C35F8 step-handler run this overlay state byte +1 indexes,
 * continuing the run after 0x800C2B48 at entry 3, and word 13 of the
 * 0x800C35D4 pointer run that holds the run. Entry 0 of the seeded pair
 * (0x800C2D2C) copies a 0xA-byte local template into its frame, calls
 * 0x8014DD3C with 0x318, clears work-record bytes +0x48 and +0x24, and finally
 * advances byte +2 of the same record by one, so the pair is this overlay next
 * two step bodies, mirroring the 0x800C2BA0/0x800C2C78 pair that the preceding
 * run entry dispatches.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C2CD4(void) {
  Boss05516Handler handlers[2] = { func_800C2D2C, advanceGlobalWorkByte1ClearWorkByte2AndCallHelperWhenEnemyReadyElseBattleUpdateAndRefreshWorkBossBoss05516_800C2E04 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
