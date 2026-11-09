#include "bof3/bof3.h"

/*
 * Scratchpad work-object pointer cell, already bound at 0x1F800044 by this
 * target's map. The symbolic address lets the assembler fold it into each load
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)), and a store through the loaded work
 * pointer may alias the cell, so the original re-materialises the base for
 * every access instead of holding one address register.
 */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/*
 * Counter table selected by the work object's unsigned byte 0xB: 12-byte
 * records whose leading three bytes are the counters this handler ramps. Each
 * counter group derives its own byte offset (idx*12 as idx*3*4) through the
 * address temporary the original spells as lui $at,%hi / addu $at,$at,$v1, so
 * the fields are reached through the three byte views of the table that the
 * other battle lifts use; the +1 and +2 relocations are the reviewed sibling
 * symbols 0x80145BD7 and 0x80145BD8 of the same table.
 */
extern u8 D_80145BD6[]; /* @source 0x80145BD6 @kind table */
extern u8 D_80145BD7[];
extern u8 D_80145BD8[];

/* @source 0x801EF1F4
 * @behavior Per-frame ramp step of this overlay's work byte 2 handler. Reads
 *           the scratchpad work-object pointer cell at 0x1F800044 and adds 1 to
 *           the work object byte at offset 0x9 with a byte-width
 *           read-modify-write (lbu / addiu 1 / sb); that stored value is not
 *           tested, so no truncation step follows it. It then adds 2 to the
 *           three counter bytes of the 12-byte counter record selected by the
 *           work object's unsigned byte 0xB: each group re-reads the cell and
 *           that work byte into its own byte offset and increments the selected
 *           0x80145BD6 record byte at offset 0x0/0x1/0x2 (lbu / addiu 2 / sb).
 *           Finally it re-reads the cell once more, reloads the work byte 0xB
 *           and, when the selected record byte at offset 0x0 equals 0x10,
 *           increments the work object byte at offset 0x2, which advances the
 *           work byte 2 dispatcher to its next handler step. That compare loads
 *           the counter with lb, so it reads the byte through the signed record
 *           view while the three increments above use the unsigned byte views;
 *           0x10 is materialised with addiu $v0,$zero,0x10. Leaf with no frame,
 *           no saved registers and no calls; takes no arguments, returns
 *           nothing and reads or writes no other state. No payload in the
 *           repository holds a byte-identical copy of these 300 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenAdd2ToWorkByteBRecordCountersAndAdvanceWorkByte2WhenCounterIs10BmagicMagic03903_801EF1F4(void) {
  u32 offset;

  g_battle_work[9]++;
  offset = g_battle_work[0xB] * 12;
  D_80145BD6[offset] += 2;
  offset = g_battle_work[0xB] * 12;
  D_80145BD7[offset] += 2;
  offset = g_battle_work[0xB] * 12;
  D_80145BD8[offset] += 2;
  if ((s8)D_80145BD6[g_battle_work[0xB] * 12] == 0x10) {
    g_battle_work[2]++;
  }
}
