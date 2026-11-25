#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F526C
 * @behavior Overlay work-record spawn: it walks the 8 entries of the
 *           0x18-byte-stride record table based at 0x800E4800 - the main-RAM
 *           work area this overlay shares with the world and scenario overlays,
 *           whose 0x18-stride byte-0 boundary this target's exact
 *           stateClearTableHead clears - and
 *           claims the first entry whose byte at offset 0 reads zero, storing 1
 *           there. It then seeds that record: byte 1 takes 0, byte 2 takes
 *           0x10, the triple at 0x04/0x08/0x0C takes the published scratch work
 *           object's words 0x34/0x38/0x3C (the object behind the scratchpad
 *           pointer cell 0x1F800044), the halfword at 0x14 takes 0x100, byte
 *           0x16 takes 0x40 and byte 0x17 takes 0. It finishes on the claim
 *           instead of continuing the walk, takes no argument, makes no call
 *           and returns nothing.
 *
 *           Consumers in this payload: byte 0 is the claim flag, because the
 *           per-frame pass func_801F5308 - still an asm span here - reads it,
 *           and clears it when the life counter runs out. Bytes 2 and 0x14 are
 *           that record's life pair: the same pass subtracts 1 from byte 2 and
 *           0x10 from the halfword at 0x14 on every active record, so the 0x10
 *           and the 0x100 written here expire together after 0x10 frames.
 *           Bytes 0x16 and 0x17 are read by the render pass func_801F53BC,
 *           which that same walk calls with the record pointer, so they stay
 *           described by offset here. The role of byte 1 is not localised by
 *           this payload either.
 *
 * Evidence: payload asm at 0x801F526C - lui/ori of 0x800E4800 into $v1 and of
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
 * Name evidence: jal 0x801F526C at 0x801F3AD0, 0x801F3B84, 0x801F3BFC and
 * 0x801F3D04, the four step handlers of this overlay's progress dispatch
 * (func_801F3AA8, func_801F3B5C, func_801F3BD4, func_801F3CDC). Each of them
 * post-increments the published work object's byte 6 and reaches this address
 * only when (byte 6 & 3) == 0, the cadence that spawns one record per 4
 * frames, and each calls func_801F5308 right after; there is no other
 * reference to this address in the payload.
 *
 * Shape evidence: the 156-byte span is byte-identical to the already-exact
 * claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 in
 * emi/world04/area188/13 (payload offsets 0x266C here and 0x11E0 there, both
 * 0x9C long) except the single absolute word at +0x74, its `j` own-immediate
 * 0x0807D4C0 against 0x0807CF9D, whose targets 0x801F5300 and 0x801F3E74
 * differ by exactly the 0x148C delta between the two spans; every other word,
 * including both lui/ori constant pairs and every relative branch, is equal, so
 * one body carries both rows. The statement order, single record pointer,
 * for-increment order (`record += 0x18u, count++`) and declaration style come
 * from that twin; this target's callers, consumers and table boundary are its
 * own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateClaimFreeRecord(void) {
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
