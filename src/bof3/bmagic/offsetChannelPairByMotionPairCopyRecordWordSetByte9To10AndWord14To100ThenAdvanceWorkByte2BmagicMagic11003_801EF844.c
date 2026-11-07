#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original re-materialises the base
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for its two access groups: the seeding run
 * at the top and the byte-2 advance at the bottom, so the named cell is
 * declared with the same spelling as the exact sibling
 * copyWorkTripletSetByte9To10AndWord14To100ThenAdvanceWorkByte2
 * BmagicMagic15103_801F1CCC. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared fixed-point channel pair in main RAM, outside this overlay's image.
 * Splat binds both addresses through this target's generated
 * undefined_syms_auto.txt (the remaining assembly of this target references the
 * same two words), so no target-map row is needed. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Shared battle work record in main RAM. The sibling lifts of this family copy
 * its word 0x3C into work word 0x3C (seedWorkWordPairAndFlagsThenAdvanceWork
 * Byte2BmagicMagic13103_801F04C0, seedMotionTintAndChannelFieldsThenAdvance
 * WorkByte2BmagicMagic00800_801F05FC). */
extern u32 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF844
 * @behavior Entry 0 of the six-word work-byte-2 handler run at 0x801F048C
 *           (0x801EF844, 0x801EF8C0, 0x801EF8F8, 0x801EF93C, 0x801F0040,
 *           0x801F00D8), which 0x801EF7B4 dispatches on scratchpad work-object
 *           byte 2; the same six words are the tail of the fourteen-word
 *           byte-1 table at 0x801F046C. It seeds the work object published at
 *           the pointer cell 0x1F800044: work words 0xC and 0x10 — the motion
 *           pair the sibling addWorkMotionPairAndIncrementBytes9AThenAdvance
 *           WorkByte2At10BmagicMagic04303_801EF55C adds into work words 0x34
 *           and 0x38 — are both set to 0x60000, work word 0x34 then takes the
 *           shared channel word at 0x80149308 plus the 0xC motion and work
 *           word 0x38 takes the shared word at 0x8014930C plus the 0x10
 *           motion, work word 0x3C takes word 0x3C of the shared battle record
 *           read through 0x801EB4E0, work byte 9 (the countdown byte the
 *           motion siblings decrement) is set to 0x10, work word 0x14 is set
 *           to 0x100 and work byte 2 is advanced with a byte-width
 *           read-modify-write. The original reads the 0xC and 0x10 words back
 *           from the work object after storing them, so the two loads the pair
 *           only recently wrote reappear instead of folding into the constant,
 *           and it re-reads the scratchpad pointer once for the byte-2
 *           advance while the earlier stores stay on the first pointer. Leaf
 *           with no frame; takes no arguments, returns nothing and reads or
 *           writes no other state. Statement order, operand forms and
 *           declaration style follow the exact siblings
 *           copyWorkTripletSetByte9To10AndWord14To100ThenAdvanceWorkByte2
 *           BmagicMagic15103_801F1CCC (same byte-9, word-0x14 and byte-2 tail)
 *           and seedWorkWordPairAndFlagsThenAdvanceWorkByte2
 *           BmagicMagic13103_801F04C0 (same channel-pair and record-word 0x3C
 *           copy). The literals 0x60000, 0x10 and 0x100 and the offsets
 *           0xC/0x10 and 0x14 appear only in this target's binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void offsetChannelPairByMotionPairCopyRecordWordSetByte9To10AndWord14To100ThenAdvanceWorkByte2BmagicMagic11003_801EF844(
    void) {
  u32 *work;
  u32 *source;

  work = (u32 *)g_battle_work;
  work[3] = 0x60000;
  work[4] = 0x60000;
  work[13] = D_80149308[0] + work[3];
  work[14] = D_8014930C[0] + work[4];
  source = (u32 *)D_801EB4E0;
  work[15] = source[15];
  ((u8 *)work)[9] = 0x10;
  work[5] = 0x100;
  g_battle_work[2]++;
}
