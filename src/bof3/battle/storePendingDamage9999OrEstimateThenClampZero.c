#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DD50
 * @behavior Decides the pending-damage halfword at byte +0x04 of the battle
 *   selection record reached through D_8014639C.record from the acting
 *   battler's work-record byte: when the 0x140-stride local-work byte at
 *   D_80145F09[D_80146374 * 0x140] is 0x0A it runs the selection-input pass
 *   func_800A3F28 and stores 9999 there, otherwise it stores the
 *   func_800A4294 estimate for kind 3 minus the 0x118-stride enemy-work
 *   halfword at 0x801EB6D6 + (D_80146394 - 3) * 0x118. The stored halfword is
 *   then clamped to zero when it is negative, through a fresh load of the
 *   selection record pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the estimate stays in a narrow s16 local. A wider s32 local
 * makes gcc convert the s16 return of func_800A4294 where it is assigned,
 * which costs an sll/sra pair the original does not have; with the diff kept
 * in the record halfword's own width the store truncates it instead. */
void storePendingDamage9999OrEstimateThenClampZero(void) {
  u32 battler;
  u32 enemy_index;
  s16 value;
  s16* record;

  battler = D_80146374;
  if (D_80145F09[battler * 0x140u] == 0x0A) {
    func_800A3F28();
    D_8014639C.record[2] = 0x270F;
  } else {
    value = func_800A4294(3);
    enemy_index = D_80146394 - 3;
    value -= *(u16*)(D_801EB6C4 + enemy_index * 0x118u + 0x12);
    D_8014639C.record[2] = value;
  }
  record = D_8014639C.record;
  if (record[2] < 0) {
    record[2] = 0;
  }
}
