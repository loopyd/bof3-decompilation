#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F3DE0
 * @behavior Overlay work-record spawn: it walks the 0x8 entries of the
 * 0x18-byte-stride record table based at 0x800E4800 - the main-RAM work area
 * this overlay shares with the world and scenario overlays, whose 0x18-stride
 * byte-0 boundary this target's exact
 * clearWorkTableByte0At800E4800World04Area18813_801F3DB4 also clears - and
 * claims the first entry whose byte at offset 0 reads zero, storing 1 there.
 * It then seeds that record: byte 1 takes 0, byte 2 takes 0x10, the triple at
 * 0x04/0x08/0x0C takes the published scratch work object's words 0x34/0x38/0x3C
 * (the object behind the scratchpad pointer cell 0x1F800044), the halfword at
 * 0x14 takes 0x100, byte 0x16 takes 0x40 and byte 0x17 takes 0. It finishes on
 * the claim instead of continuing the walk, takes no argument, makes no call
 * and returns nothing.
 *
 * Byte 0 is the claim flag: the walk here tests it, and this target's per-frame
 * pass func_801F3E7C skips inactive records on it and clears it when the
 * record's life runs out. Bytes 2 and 0x14 are that record's life pair: the
 * same pass subtracts 1 from byte 2 and 0x10 from the halfword at 0x14 on every
 * active record, so the 0x10 and the 0x100 written here expire together after
 * 0x10 frames; the halfword is the extent the render pass
 * drawTranslucentGouraudFanOfThirtyTwoTrianglesAroundProjectedWorkRecordPositionAtConvertedExtentTintedByCentreAndRimBytesWorld04Area18813_801F3F30
 * converts at the depth of the position triple at 0x04 and uses as the radius
 * of the translucent fan it draws around that projected point. Bytes 0x16 and
 * 0x17 are that pass's two tints: the 0x40 written here lights the shared
 * centre vertex of the fan and the 0 written here leaves the two rim vertices
 * dark. The role of byte 1 is still not localised by this payload, so it stays
 * described by offset.
 *
 * Evidence: payload asm at 0x801F3DE0 - lui/ori of 0x800E4800 into $v1 and of
 * 0x800E4817 into $a0, the loop-invariant literals 1/0x10/0x100/0x40 hoisted
 * into $a2/$a3/$t0/$t1, then per record lbu $v0,0x0($v1) and bnez to the
 * advance block, sb $a2,0x0($v1), sb $zero,-0x16($a0), sb $a3,-0x15($a0),
 * lui/lw of the 0x1F800044 cell into $v1, lw 0x34/0x38/0x3C of that object into
 * the word stores at -0x13/-0xF/-0xB($a0), sh 0x100 at -0x3($a0), sb 0x40 at
 * -0x1($a0), sb $zero,0x0($a0) and j to the epilogue with the last word store
 * in its delay slot. The advance block adds 0x18 to $a0, adds 1 to $a1, masks
 * $a1 to 0xFF, compares it below 0x8 and adds 0x18 to $v1 in the back edge's
 * delay slot, so the walk is an unsigned byte counter over one record pointer.
 *
 * Name evidence: the only in-image references to this address are jal
 * 0x801F3DE0 at 0x801F35C0, 0x801F3674, 0x801F36EC and 0x801F37F4, the four
 * step handlers of this overlay's progress dispatch, each of which advances the
 * published work object's byte 6 and reaches this address only when
 * (byte 6 & 3) == 0 - the cadence that spawns one record per 4 frames - before
 * that step's per-frame pass calls func_801F3E7C, the pass that ticks and draws
 * the active records. The claim-then-seed shape and the position triple follow
 * the scenario overlay's exact
 * claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C
 * and this repository's claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0;
 * this target's own table base, stride, literals and offsets come from the
 * original bytes.
 *
 * Matching note: the two addressing anchors in the original are one pointer,
 * not two. GCC's loop optimisation keeps the record pointer itself for the
 * flag read and the claim store and derives a second induction register
 * anchored at record + 0x17 for the seed group, whose every store then reads as
 * a negative displacement - writing the seed through a hand-built
 * `record + 0x17` local instead adds a third anchor, an extra literal
 * materialisation and four bytes. The cell read is the other decisive shape:
 * through `SPAD_PTR_SLOT`/`SPAD_PTR_TABLE` the 0x1F800044 address is materialised
 * with lui + ori and loaded at displacement 0, while the named cell symbol folds
 * to lui 0x1F80 plus `lw 0x44(reg)`, which is the original's pair and four bytes
 * smaller. The for-increment order is load bearing too: `record += 0x18u,
 * count++` emits the anchor advance before the counter advance, and swapping
 * them loses one matched instruction at the advance block.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0(void) {
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
    record[2] = 0x10;
    work = D_1F800044;
    *(u32*)(record + 0x04) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x08) = *(u32*)(work + 0x38);
    *(u32*)(record + 0x0C) = *(u32*)(work + 0x3C);
    *(u16*)(record + 0x14) = 0x100;
    record[0x16] = 0x40;
    record[0x17] = 0;
    return;
  }
}
