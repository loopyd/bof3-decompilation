#include "bof3/bof3.h"
#include "data/equipment.h"

/* The shared level table at 0x801CB8DC: 99 eight-byte entries per character
 * row, each starting with the u16 experience threshold of that level. The row
 * stride is written out because the original computes `row * 99 + index` per
 * iteration instead of walking a row pointer. */
extern LevelObject D_801CB8DC[]; /* @source 0x801CB8DC @kind table */

/* Level byte +0x06 and experience word +0x08 of the 164-byte (0xA4) records
 * that start at 0x80144968. The original reaches each through its own 32-bit
 * displacement (`lui $at,%hi(X)` + `addu $at,$at,row*0xA4` + load
 * `%lo(X)($at)`), so both are bound as flat symbols at their own addresses
 * instead of as one record. */
extern u8 D_8014496E[]; /* @source 0x8014496E @kind unknown */
extern u8 D_80144970[]; /* @source 0x80144970 @kind unknown */

/* Kind byte +0x79 of the 0x140-byte battle work records that run from
 * 0x80145E90; the original reaches the byte of record `battler` as
 * 0x80145F09 + battler * 0x140, so it is bound as a flat byte run. */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind unknown */

/* The EXE state-byte lookup at 0x801DD7AC (lifted as lookupStateByteMapped in
 * the battle/03 target): returns one indexed state byte and maps value seven
 * to zero. Outside this overlay, so it is declared here for its address. */
extern u8 func_801DD7AC(s32 arg0);

/* @source 0x801F0BB0
 * @behavior Remaining-experience readout for one local battler: takes byte
 *   +0x79 of the battler's 0x140-byte battle work record, maps it through the
 *   EXE state-byte lookup func_801DD7AC (value seven becomes zero) and uses the
 *   result as the character row of the shared level tables. It sums the u16
 *   thresholds of D_801CB8DC for that row from index zero through N inclusive,
 *   where N is byte +0x06 of the row's 164-byte record and is re-read from
 *   D_8014496E on every iteration. It compares the sum with the u32 experience
 *   at +0x08 (D_80144970): the sum minus that experience is returned when the
 *   sum is the larger, and zero otherwise. The sum loop runs its body before
 *   the first test, so a zero level byte still sums entry 0. The 0x18-byte
 *   frame keeps $ra and $s0 across the func_801DD7AC call.
 * @status partial
 * @match 77.59
 * @residual 45/58 instructions, 232->232 bytes. Separate loop and post-loop
 *   row lifetimes retain the per-iteration index calculation and fresh tail
 *   offset calculation. The first mismatch is at +0x34: the current code
 *   narrows the lookup result before copying it, then narrows the loop row
 *   again inside the loop. Row/index allocation and increment scheduling also
 *   differ. The inclusive backedge and the post-loop saturation match.
 */
s32 sumRemainingLevelExpBatlEnd00_801F0BB0(u8 battler) {
  u8 i;
  s32 selected;
  s32 total;
  u32 experience;

  total = 0;
  selected = func_801DD7AC(D_80145F09[battler * 320]);
  i = 0;
  {
    u8 row = selected;
again:
    total += D_801CB8DC[row * 99 + i].exp;
    i += 1;
    if (i <= D_8014496E[row * 164]) {
      goto again;
    }
  }
  experience = *(u32 *)(D_80144970 + (u8)selected * 164);
  if ((u32)total < experience) {
    return 0;
  }
  return total - experience;
}
