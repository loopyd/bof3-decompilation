#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void applyMainLookupRecord318ClearWorkByte48Byte24AndWord3CCopyTemplate1840ThenRequestByteAtIndexedByBattleHalfwordCallEnemyReadyBattleUpdateAndRefreshWorkThenAdvanceWorkByte2BossBoss05516_800C2ECC(void);
void callHelperWhenGlobalWorkByte1Is2ElseCallEnemyReadyBattleUpdateAndRefreshWorkBossBoss05516_800C2FA4(void);

/* @source 0x800C2E74
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C2ECC then 0x800C2FA4 and calls the entry selected by
 * the record's unsigned byte +2 without arguments. The original reads the
 * work-record pointer once ahead of both frame-slot stores, leaves the index
 * unbounded and writes no state of its own: the 0x20-byte frame only keeps $ra
 * across the indirect call. Its body is the same 22 instructions and 0x58 bytes
 * as the exact siblings 0x800C2728, 0x800C2888, 0x800C29E8, 0x800C2B48 and
 * 0x800C2CD4 and differs from them only in the pair it seeds. The word at
 * 0x800C360C holds this function address and no other word of the payload does,
 * so it is entry 8 of the 0x800C35EC handler run this overlay indexes, following
 * 0x800C2CD4 at entry 7. Entry 0 of the seeded pair (0x800C2ECC) copies the
 * 0xA-byte template at 0x800C1840 into its frame, calls 0x8014DD3C with 0x318,
 * clears work-record bytes +0x48 and +0x24 and word +0x3C, runs the EXE-side
 * helpers and finally advances byte +2 of the same record by one, so the pair is
 * this overlay's next two step bodies. Entry 1 (0x800C2FA4) is the steady-state
 * body: it reads byte +1 of the object published at 0x801EB4E0 and runs either
 * the 0x801E5988 path or the 0x801E30F8/0x8014D4E0/0x800C3284 trio.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C2E74(void) {
  Boss05516Handler handlers[2] = { applyMainLookupRecord318ClearWorkByte48Byte24AndWord3CCopyTemplate1840ThenRequestByteAtIndexedByBattleHalfwordCallEnemyReadyBattleUpdateAndRefreshWorkThenAdvanceWorkByte2BossBoss05516_800C2ECC, callHelperWhenGlobalWorkByte1Is2ElseCallEnemyReadyBattleUpdateAndRefreshWorkBossBoss05516_800C2FA4 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
