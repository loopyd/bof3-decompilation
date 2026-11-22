#include "bof3/bof3.h"

/*
 * Scratchpad work-object pointer cell bound at 0x1F800044 by this target's map.
 * The symbolic address lets the assembler fold the cell into each load
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)). The original re-materialises the base
 * once per counter group and once more for the compare tail, so a store through
 * the loaded work pointer never holds one address register across the groups.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's unsigned byte 0xA: 12-byte
 * records whose leading three bytes are the counters this handler ramps. Each
 * group derives its own byte offset (idx*12 as idx*3*4) through the address
 * temporary the original spells as lui $at,%hi / addu $at,$at,$v1, so the three
 * fields are reached through the three byte views of the table that the other
 * bmagic battle lifts use. 0x80145BD6 and 0x80145BD8 still arrive from this
 * target's generated undefined_syms_auto.txt through the remaining asm span
 * func_801EF834, and 0x80145BD7 binds from its address-encoded name.
 */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EF724
 * @behavior Per-frame handler of this overlay's handler run: it reads the
 * scratchpad work-object pointer cell at 0x1F800044 once per counter group and,
 * for each of the record's three counter bytes, re-reads the work object's
 * unsigned byte 0xA, scales it by the 12-byte record stride into a byte offset
 * and increments the selected 0x80145BD6 record's byte 0x0/0x1/0x2 with a
 * byte-width read-modify-write (lbu, addiu $v0,$v0,1, sb). It then re-reads the
 * cell once more into a second address register, reloads the work byte 0xA and
 * loads the selected record's byte 0 through the signed view (lb) before
 * comparing it with 0x10, which is materialised as addiu $v0,$zero,0x10 and
 * tested with bne. Only when the stored counter equals 0x10 does it increment
 * the work object's byte 2 with the same byte-width add, advancing the work
 * byte 2 dispatcher to its next handler step. The compare re-uses the single
 * cell load of its own group for that final increment, while the three counter
 * groups each re-read the cell. Leaf with no frame, no saved registers and no
 * calls; takes no arguments, returns nothing and reads or writes no other
 * state. The counter triplet copies the statement order of the exact siblings
 * incrementWorkByteARecordCountersThenDecrementWorkByte9AndAdvanceWorkByte2On
 * ZeroBmagicMagic21303_801EF23C and incrementWorkByte9ThenAdd2ToWorkByteBRecord
 * CountersAndAdvanceWorkByte2WhenCounterIs10BmagicMagic03903_801EF1F4, whose
 * three byte-width read-modify-writes and signed 0x10 compare on this same
 * table match; this span writes no countdown byte and increments each counter by
 * one, and no payload in the repository holds a byte-identical copy of these
 * 0x110 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteARecordCountersThenAdvanceWorkByte2WhenCounterIs10BmagicMagic06903_801EF724(void) {
  u32 offset;

  offset = g_battle_work[0xA] * 12;
  D_80145BD6[offset]++;
  offset = g_battle_work[0xA] * 12;
  D_80145BD7[offset]++;
  offset = g_battle_work[0xA] * 12;
  D_80145BD8[offset]++;
  if ((s8)D_80145BD6[g_battle_work[0xA] * 12] == 0x10) {
    g_battle_work[2]++;
  }
}
