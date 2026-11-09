#include "bof3/bof3.h"

void func_801E3160(void);

/* Shared battle flag halfword the main program keeps at 0x801462E8: bit 0x800
 * is the pending-enemy flag the battle step code at 0x800ADFBC dispatches on. */
extern u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Battle global byte at 0x801463C8 that the shared battle code subtracts from
 * enemy work halfword +0x96; the EXE-side 0x801E4E10 folds the same halfword by
 * the same byte. */
extern u8 D_801463C8; /* @source 0x801463C8 @kind unknown */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad pointer cell holding this overlay work record. Declared as the
 * absolute-address symbol so the use reloads the cell with the lui plus offset
 * load form the original shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x800C2158
 * @behavior Overlay state body, reached as index 1 of the handler run at
 * 0x800C35D8 that dispatchStateByte2ThenAdvanceOnState8BossBoss05516_800C20A4
 * indexes by the scratchpad work object's state byte +2. It calls the EXE-side
 * enemy-readiness helper at 0x801E3160 first, unconditionally and discarding
 * the result, then, when the current enemy work record published at
 * 0x801EB4E8 carries mode byte +0xF5 == 4, folds that record's unsigned
 * halfword +0x96 down by the global byte 0x801463C8. On every path it then
 * raises bit 0x800 of the shared battle flag halfword 0x801462E8, clears the
 * record's mode byte +0xF5 and advances the scratchpad work object's state
 * byte +2 by one, so the next dispatch of the same run enters the following
 * state body. Nothing but $ra lives in the 0x18-byte frame, and the record
 * pointer cell is re-read after the conditional store because that store may
 * alias the cell. The EXE-side 0x801E4E10 runs the same fold and the same flag
 * and mode stores, but calls the readiness helper last and only when the kind
 * mask selected by 0x801463C0 lacks bit 0x800, so this body is the overlay copy
 * that front-loads that call; the roles of mode value 4 and of halfword +0x96
 * are not established beyond that shared use.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callEnemyReadyThenFoldHalfword96RaiseFlag800ClearModeByteAndAdvanceWorkByte2BossBoss05516_800C2158(void) {
  u16 *flags;

  func_801E3160();

  if (FIELD_REF(u8, D_801EB4E8, 0xF5u) == 4u) {
    FIELD_REF(u16, D_801EB4E8, 0x96u) -= D_801463C8;
  }

  flags = &D_801462E8;
  *flags |= 0x800u;

  FIELD_REF(u8, D_801EB4E8, 0xF5u) = 0u;

  g_battle_work[2]++;
}
