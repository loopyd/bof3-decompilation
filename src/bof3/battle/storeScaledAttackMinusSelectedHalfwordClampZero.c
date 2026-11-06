#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0D64
 * @behavior Selection-action handler, entry 104 of the battle selection-action
 *   table D_800B471C. Stores into the signed halfword at byte +0x04 of the
 *   selection record pointer D_8014639C.record the value func_800A2880 scales
 *   from the 0x14-row table byte D_801CA71B[D_801463C0 * 0x14] for the active
 *   battler byte D_80146374 and the selected battler byte D_80146394 with
 *   element flag 0, subtracts the selected battler's work-record halfword - the
 *   0x140-stride party work record halfword at 0x80145F26 while that byte is
 *   below 3, otherwise the 0x118-stride enemy work record halfword at
 *   0x801EB6C4 + 0x12 selected by the byte rebased by -3 - and clamps the
 *   stored halfword to zero when it is negative.
 * @status partial
 * @match 98.46
 * @residual first=+0x0000: the +0x18 frame adjust is scheduled at entry instead
 *   of after the three leading global loads; every other instruction, the size
 *   (260 bytes) and the order of the remaining body match.
 */
/* Matching notes. Two shapes were measured here. The result store must be a
 * volatile halfword write: through a plain `s16 *` view gcc hoists the
 * post-call re-read of D_80146394 above the store and both load-delay slots of
 * the original tail disappear. The post-call re-read is also wider than a byte
 * local: read through the header's `volatile u8` cell the assignment to a wide
 * local costs an `andi 0xff`, which the original does not have, so the byte is
 * read through a non-volatile view of the same address. The enemy arm needs the
 * rebased index in its own u32 local: written inline gcc distributes the -3
 * into the record displacement and drops the addiu rebase, which is the shape
 * the exact kind-1 sibling measured. */
void storeScaledAttackMinusSelectedHalfwordClampZero(void) {
  u16 index;
  u32 rebased;
  u32 selection;
  s16* record;

  index = D_801463C0;
  /* The index leaves D_801463C0 before the call arguments are read, so the
   * leading loads stay on their named cells instead of a shared base. */
  *(volatile s16 *)&D_8014639C.record[2] =
      func_800A2880(D_80146374, D_80146394, D_801CA71B[index * 0x14], 0);
  selection = *(u8 *)&D_80146394;
  if (selection < 3) {
    D_8014639C.record[2] -= *(u16*)(D_80145F26 + selection * 0x140u);
  } else {
    rebased = selection - 3;
    D_8014639C.record[2] -= *(u16*)(D_801EB6C4 + rebased * 0x118u + 0x12);
  }
  record = D_8014639C.record;
  if (record[2] < 0) {
    record[2] = 0;
  }
}
