#include "bof3/bof3.h"

typedef void (*Boss05516Handler)(void);

void func_800C2BA0(void);
extern void callEnemyReadyThenCallHelperWhenGlobalWorkByte1Is2ElseBattleUpdateAndRefreshWorkBossBoss05516_800C2C78(void); /* @source 0x800C2C78 */

/* @source 0x800C2B48
 * @behavior Overlay step dispatch on the state byte of the work record
 * published at scratchpad pointer cell 0x1F800044: seeds a local two-entry
 * handler array with 0x800C2BA0 then 0x800C2C78, and calls the entry selected
 * by the record's unsigned byte +2 without arguments. The original reads the
 * work-record pointer once ahead of both frame-slot stores, leaves the index
 * unbounded and writes no state of its own: the 0x20-byte frame only keeps $ra
 * across the indirect call. Its body is the same 22 instructions and 0x58 bytes
 * as the exact siblings 0x800C2728, 0x800C2888 and 0x800C29E8 and differs from
 * them only in the pair it seeds. The word at 0x800C3604 holds this function's
 * address and no other word of the payload does, so it is entry 3 of the
 * 0x800C35F8 step-handler run this overlay's state byte +1 indexes, continuing
 * the run after 0x800C29E8 at entry 2. Entry 0 of the seeded pair (0x800C2BA0)
 * performs setup, calls the EXE-side helpers, then advances byte +2 of the same
 * record by one, so the pair is this overlay's next two step bodies, mirroring
 * the 0x800C2A40/0x800C2AA8 pair that the preceding run entry dispatches.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2LocalPairBossBoss05516_800C2B48(void) {
  Boss05516Handler handlers[2] = { func_800C2BA0, callEnemyReadyThenCallHelperWhenGlobalWorkByte1Is2ElseBattleUpdateAndRefreshWorkBossBoss05516_800C2C78 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
