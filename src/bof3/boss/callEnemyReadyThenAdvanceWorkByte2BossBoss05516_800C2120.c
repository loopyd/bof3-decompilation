#include "bof3/bof3.h"

void func_801E3160(void);

/* Scratchpad pointer cell holding this overlay work record. Declared as the
 * absolute-address symbol so the use reloads the cell with the lui plus offset
 * load form the original shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x800C2120
 * @behavior Overlay state body, reached as index 0 of the handler run at
 * 0x800C35D8 that dispatchStateByte2ThenAdvanceOnState8BossBoss05516_800C20A4
 * indexes by the scratchpad work object's state byte +2. It calls the EXE-side
 * enemy-readiness helper at 0x801E3160 first, unconditionally and discarding
 * the result, then loads the work record pointer published at 0x1F800044 and
 * advances that record's unsigned byte +2 by one, so the next dispatch of the
 * same run enters the following state body. Nothing but $ra lives in the
 * 0x18-byte frame, the pointer cell is read once after the call and its value
 * is reused for both the byte load and the byte store.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEnemyReadyThenAdvanceWorkByte2BossBoss05516_800C2120(void) {
  func_801E3160();
  g_battle_work[2]++;
}
