#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44, so the cell is its own
 * fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F38E8
 * @behavior Overlay work-record spawn: it walks the 8 entries of the
 *           0x18-byte-stride record table based at 0x800E4800 - the main-RAM
 *           work area this overlay shares with the world and scenario overlays,
 *           whose 0x18-stride lead-byte boundary this target's exact
 *           clearWorkTableAt800E4800World04Area17213_801F38BC also clears - and
 *           claims the first entry whose byte at offset 0 reads zero, storing 1
 *           there. It then seeds that record: byte 1 takes 0, byte 2 takes
 *           0x20, the triple at 0x04/0x08/0x0C takes the published scratch work
 *           object's words 0x34/0x38/0x3C (the object behind the scratchpad
 *           pointer cell 0x1F800044), the halfword at 0x14 takes 0x180, byte
 *           0x16 takes that object's byte 0x5D and byte 0x17 takes 0. It
 *           finishes on the claim instead of continuing the walk, takes no
 *           argument, makes no call and returns nothing.
 *
 * Byte 0 is the claim flag: this walk tests it, and this target's per-frame
 * record pass 0x801F3984 skips inactive records on it and clears it when the
 * record's countdown runs out. Byte 2 is that countdown and the halfword at
 * 0x14 is the pair it expires with: the same pass subtracts 1 from byte 2 and
 * 0x0C from the halfword at 0x14 on every active record, so the 0x20 frame
 * countdown and the 0x180 halfword run out together after 0x20 frames. The
 * countdown, the position triple and the halfword are the fields the pass
 * re-reads; byte 1 and bytes 0x16/0x17 are not touched by it, and byte 1 has
 * no established role in this payload, so it stays described by offset. Byte
 * 0x5D of the work object is the lead byte of the signed level triple
 * 0x5D/0x5E/0x5F that this overlay's
 * armScratchWorkLevelTripleWorld04Area17213_801F4AAC and
 * stateLevelAccumulatorsMarker
 * step, so byte 0x16 receives the work object's current level.
 *
 * Evidence: payload asm at 0x801F38E8 - lui/ori of 0x800E4800 into $v1 and of
 * 0x800E4817 into $a0, the loop-invariant literals 1/0x20/0x180 hoisted into
 * $a2/$a3/$t0, then per record lbu $v0,0x0($v1) and bnez to the advance block,
 * sb $a2,0x0($v1), sb $zero,-0x16($a0), sb $a3,-0x15($a0), lui/lw of the
 * 0x1F800044 cell into $v1, lw 0x34/0x38/0x3C of that object into the word
 * stores at -0x13/-0xF/-0xB($a0), sh 0x180 at -0x3($a0), lbu 0x5D of that
 * object, sb $zero,0x0($a0) and sb of the loaded level at -0x1($a0) before the
 * jump to the epilogue. The advance block adds 0x18 to $a0, adds 1 to $a1,
 * masks $a1 to 0xFF, compares it below 0x8 and adds 0x18 to $v1 in the back
 * edge's delay slot, so the walk is an unsigned byte counter over one record
 * pointer. The span is 0x9C bytes with no frame and no call.
 *
 * Shape evidence: a whole-image search of out/binaries for these 0x9C bytes
 * finds this payload alone, so the clone lever is declined and this target owns
 * its source and boundary. The nearest exact siblings are the same-shaped seeds
 * claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 and
 * claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C, which share
 * the table, stride, entry count, claim order and the 0x04/0x08/0x0C position
 * copy; they differ in the stored literals, in byte 0x16 taking a literal there
 * and this work object's level byte here, and in this span's last two stores,
 * whose order this body takes from its own bytes.
 *
 * Matching note: the two addressing anchors in the original are one pointer,
 * not two. GCC's loop optimisation keeps the record pointer itself for the flag
 * read and the claim store and derives a second induction register anchored at
 * record + 0x17 for the seed group, whose every store then reads as a negative
 * displacement; writing the seed through a hand-built `record + 0x17` local
 * adds a third anchor, an extra literal materialisation and four bytes. The
 * cell read is the other decisive shape: through a named cell symbol the
 * 0x1F800044 address folds to lui 0x1F80 plus `lw 0x44(reg)`, which is the
 * original's pair. The for-increment order is load bearing too: `record +=
 * 0x18u, count++` emits the anchor advance before the counter advance.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreePositionLevel(
    void) {
  u8* record;
  u8* work;
  u8 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x8u; record += 0x18u, count++) {
    if (*record != 0) {
      continue;
    }
    *record = 1;
    record[1] = 0;
    record[2] = 0x20;
    work = D_1F800044;
    *(u32*)(record + 0x04) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x08) = *(u32*)(work + 0x38);
    *(u32*)(record + 0x0C) = *(u32*)(work + 0x3C);
    *(u16*)(record + 0x14) = 0x180;
    record[0x16] = *(u8*)(work + 0x5D);
    record[0x17] = 0;
    return;
  }
}
