#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui plus an offset load, so the cell is its own fixed-address symbol. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Table of 0x10 records of 0x18 bytes at 0x801D39A8 inside this overlay. The
 * lead byte of each record is its claim flag. The bytes sit inside the Splat
 * `textbin` row at payload offset 0x2D18 and are zero on disc, so the run is
 * only referenced here. */
extern u8 D_801D39A8[]; /* @source 0x801D39A8 @kind data */

/* @source 0x801D19C8
 * @behavior Overlay work-record spawn: it walks the 0x10 entries of the
 * 0x18-byte-stride record table at 0x801D39A8 and claims the first entry whose
 * byte at offset 0 reads zero, storing 1 there. It then seeds that record: byte
 * 1 takes 0, byte 2 takes 0x10, the triple at 0x04/0x08/0x0C takes the
 * published scratch work object's words 0x34/0x38/0x3C (the object behind the
 * scratchpad pointer cell 0x1F800044), the halfword at 0x14 takes 0x100, byte
 * 0x16 takes 0x60 and byte 0x17 takes 0. It finishes on the claim instead of
 * continuing the walk, takes no argument, makes no call, returns nothing and
 * carries no frame and no saved register.
 *
 * Byte 0 is the claim flag and bytes 2 and 0x14 the record's life pair: this
 * target's next function func_801D1A60 walks the same table, skips every
 * record whose lead byte reads zero, and for an active record subtracts 1 from
 * byte 2 and 0x10 from the halfword at 0x14 on every frame, clearing the lead
 * byte when byte 2 reaches zero, so the 0x10 and the 0x100 written here expire
 * together after 0x10 frames. The words at 0x04/0x08/0x0C are a snapshot of
 * the work object's position triple: the only site that calls this address
 * twice, jal 0x801D19C8 at 0x801D12C8 and 0x801D1318 inside func_801D12A0,
 * swaps the work words 0x34/0x38/0x3C with 0x18/0x1C/0x20 between the two
 * calls. The roles of byte 1, byte 0x16 and byte 0x17 are not localised by this
 * payload, so they stay described by offset.
 *
 * Evidence: payload asm at 0x801D19C8 - lui/addiu of D_801D39A8 into $v1 and
 * addiu $a0,$v1,0x17, the loop-invariant literals 1/0x10/0x100/0x60 hoisted into
 * $a2/$a3/$t0/$t1, then per record lbu $v0,0x0($v1) and bnez to the advance
 * block, sb $a2,0x0($v1), sb $zero,-0x16($a0), sb $a3,-0x15($a0), lui/lw of the
 * 0x1F800044 cell into $v1, lw 0x34/0x38/0x3C of that object into the word
 * stores at -0x13/-0xF/-0xB($a0), sh 0x100 at -0x3($a0), sb 0x60 at -0x1($a0),
 * sb $zero,0x0($a0) and j to the epilogue with the last word store in its delay
 * slot. The advance block adds 0x18 to $a0, adds 1 to $a1, masks $a1 to 0xFF,
 * compares it below 0x10 and adds 0x18 to $v1 in the back edge's delay slot, so
 * the walk is an unsigned byte counter over one record pointer.
 *
 * Name evidence: the five in-image references are jal 0x801D19C8 at 0x801D0F38,
 * 0x801D1014, 0x801D109C, 0x801D12C8 and 0x801D1318, all of which pass no
 * argument and discard the result. Four of them - the three step frames
 * func_801D0F10, func_801D0FEC and func_801D106C and the first of the two calls
 * in func_801D12A0 - reach this address only when (work byte 6 & 3) == 0 after
 * advancing that byte, the cadence that spawns one record per 4 frames. Byte 0
 * as the claim flag and the 0x34/0x38/0x3C triple as the position follow this
 * target's exact
 * clearRecordActiveFlagsAt801D39A8ScenarioSce15ef100_801D199C, which names the
 * same 0x18-stride table at 0x801D39A8 and records this function as the one
 * that sets the lead byte an overwritten pass clears.
 *
 * Shape evidence: bytes 0x11E0..0x127B of emi/world04/area188/13 are this
 * body's sibling with another table base, entry count and re-arm constant - its
 * exact lift claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0
 * shows the identical statement order, the identical two-register walk and the
 * identical -0x16..0x0 displacements from the record+0x17 anchor. Its matching
 * note records that the two addressing anchors are one pointer: GCC's loop
 * optimisation keeps the record pointer for the flag read and the claim store
 * and derives the record+0x17 induction register for the seed group, so writing
 * the seed through a hand-built `record + 0x17` local adds a third anchor and
 * extra bytes. That record also settles the cell spelling: through
 * `SPAD_PTR_SLOT` the 0x1F800044 address is materialised with lui + ori and
 * loaded at displacement 0, while the named cell symbol folds to this
 * original's lui 0x1F80 plus `lw 0x44(reg)`.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordSeedPositionAt801D39A8ScenarioSce15ef100_801D19C8(void) {
  u8* record;
  u8* work;
  u8 count;

  record = D_801D39A8;

  for (count = 0u; count < 0x10u; record += 0x18u, count++) {
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
    record[0x16] = 0x60;
    record[0x17] = 0;
    return;
  }
}
