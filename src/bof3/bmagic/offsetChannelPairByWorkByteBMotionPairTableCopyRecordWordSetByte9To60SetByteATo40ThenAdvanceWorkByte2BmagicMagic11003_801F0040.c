#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the base
 * (lui $vN,0x1F80 / lw $vN,68($vN)) three times: once for the seeding run at the
 * top and once for each of the two byte stores at the bottom, so the named cell
 * is declared with the spelling of the exact sibling
 * offsetChannelPairByMotionPairCopyRecordWordSetByte9To10AndWord14To100Then
 * AdvanceWorkByte2BmagicMagic11003_801EF844. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared fixed-point channel pair in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map binds both addresses because the
 * remaining assembly of the target reads the same two words, so no target-map
 * row is needed. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Shared battle work record in main RAM. The sibling lifts of this family copy
 * its word 0x3C into work word 0x3C. */
extern u32 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Fixed-point offset-pair table: row i is the motion pair the original adds to
 * the shared channel pair, selected by work byte 0xB. It sits inside the data
 * span the target's Splat labels 0x801F046C, right after the fourteen handler
 * words that span holds, and the address encodes in its name, so it binds with
 * no target-map row. */
/* @source 0x801F04A4 @kind table */
extern u32 D_801F04A4[][2];

/* @source 0x801F0040
 * @behavior Entry 4 of the six-word work-byte-2 handler run at 0x801F048C
 *           (0x801EF844, 0x801EF8C0, 0x801EF8F8, 0x801EF93C, 0x801F0040,
 *           0x801F00D8), which 0x801EF7B4 dispatches on scratchpad work-object
 *           byte 2; the same six words are the tail of the fourteen-word
 *           byte-1 table at 0x801F046C. It seeds the work object published at
 *           the pointer cell 0x1F800044: work word 0x34 — the fixed-point
 *           channel word the motion siblings add into — takes the shared channel
 *           word at 0x80149308 plus the first word of row 0xB of the offset-pair
 *           table at 0x801F04A4, and work word 0x38 takes the shared word at
 *           0x8014930C plus that row's second word. Work word 0x3C then takes
 *           word 0x3C of the shared battle record read through 0x801EB4E0, work
 *           byte 9 (the countdown byte the motion siblings decrement) is set to
 *           0x60, work byte 0xA is set to 0x40, and work byte 2 is advanced with
 *           a byte-width read-modify-write. The original reads work byte 0xB
 *           twice, once per channel word, and re-reads the scratchpad pointer
 *           cell for each of the two byte stores at the bottom. Leaf with no
 *           frame; takes no arguments, returns nothing and reads or writes no
 *           other state. Statement order, operand forms and declaration style
 *           follow the exact sibling
 *           offsetChannelPairByMotionPairCopyRecordWordSetByte9To10AndWord14To100
 *           ThenAdvanceWorkByte2BmagicMagic11003_801EF844, which holds the same
 *           channel-pair and record-word 0x3C copy plus the byte-2 tail. The
 *           pair-table base 0x801F04A4 and the literals 0x60 and 0x40 appear
 *           only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void offsetChannelPairByWorkByteBMotionPairTableCopyRecordWordSetByte9To60SetByteATo40ThenAdvanceWorkByte2BmagicMagic11003_801F0040(
    void) {
  u32 *work;
  u32 *source;

  work = (u32 *)g_battle_work;
  work[13] = D_80149308[0] + D_801F04A4[((u8 *)work)[0xB]][0];
  work[14] = D_8014930C[0] + D_801F04A4[((u8 *)work)[0xB]][1];
  source = (u32 *)D_801EB4E0;
  work[15] = source[15];
  ((u8 *)work)[9] = 0x60;
  g_battle_work[0xA] = 0x40;
  g_battle_work[2]++;
}
