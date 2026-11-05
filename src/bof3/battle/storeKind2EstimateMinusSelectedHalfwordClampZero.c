#include "bof3/battle/battle15_internal.h"

/* @source 0x800A243C
 * @behavior Selection-action handler, entry 124 of the battle selection-action
 *   table D_800B471C. Stores into the signed halfword at byte +0x04 of the
 *   selection record pointer D_8014639C.record the func_800A4294 estimate for
 *   request kind 2 minus the selected battler's work-record halfword: for an
 *   active battler byte D_80146394 below 3 that is the 0x140-stride party work
 *   record halfword at 0x80145F26, otherwise the 0x118-stride enemy work record
 *   halfword at 0x801EB6C4 + 0x12 selected by that byte rebased by -3. The
 *   battler byte is re-read after the estimate call in each arm, and the stored
 *   halfword is clamped to zero when negative through a fresh load of the
 *   selection record pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: this body is the kind-2 twin of the exact sibling
 * storeKind1EstimateMinusSelectedHalfwordClampZero: word-for-word identical
 * except the estimate kind literal, so that source's shape carries over whole.
 * The battler byte feeds a plain volatile pointer so one base register serves
 * the two post-call reloads. The two arms are deliberately not symmetric. In
 * the party arm the battler byte is read inline in the store expression, which
 * is what lets the estimate call result stay in its own return register through
 * the multiply chain. The enemy arm needs the rebased index in its own u32
 * local: written inline, gcc distributes the -3 into the record displacement
 * (`lhu %lo`, 0x348 lower) and replaces the `addiu` rebase with an `andi` mask.
 * Both arms store the same record halfword, which gcc cross-jumps into the one
 * post-join store the original shows. */
void storeKind2EstimateMinusSelectedHalfwordClampZero(void) {
  volatile u8* selection;
  u32 index;
  s16 value;
  s16* record;

  selection = &D_80146394;
  if (*selection < 3) {
    D_8014639C.record[2] =
        func_800A4294(2) - *(u16*)(D_80145F26 + (*selection) * 0x140u);
  } else {
    value = func_800A4294(2);
    index = *selection - 3;
    D_8014639C.record[2] = value - *(u16*)(D_801EB6C4 + index * 0x118u + 0x12);
  }
  record = D_8014639C.record;
  if (record[2] < 0) {
    record[2] = 0;
  }
}
