#include "bof3/battle/battle15_internal.h"

/* @source 0x800A3638
 * @behavior returns 0 when battler_index is below 3 and value equals either
 * current local work byte 0x86 or 0x87; otherwise returns 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 passesBytePairGate(u8 battler_index, u8 value) {
  BattleLocalWork* work;

  if (battler_index < 3) {
    work = (BattleLocalWork*)D_80146250;
    if (work->unk_86 == value || work->unk_87 == value) {
      return 0;
    }
  }
  return 1;
}
