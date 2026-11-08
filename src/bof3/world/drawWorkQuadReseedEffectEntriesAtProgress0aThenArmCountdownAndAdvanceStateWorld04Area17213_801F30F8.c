#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row. The original re-materialises the cell with a `lui 0x1F80` plus a load
 * at displacement 0x44 for every one of the five access groups below, one group
 * per statement, so the cell is its own fixed-address symbol rather than the
 * constant-address accessor. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared scenario progress byte at 0x80146864, bound by the shared map. The body
 * reads it through the plain symbol expression, which is the `lui $v1,%hi` plus
 * folded `lbu $v1,%lo` pair the original bytes show. */
extern u8 g_ScenarioProgress;

/* Work quad emitter at 0x801F36C0: it asks for the 0x2C0/0x100 tpage, sets the
 * draw mode and the semi-transparency, builds the panel transform from the work
 * object's 0x34/0x38/0x3C position words and its 0x2E angle halfword, emits the
 * four-corner shaded quad into the shared primitive cursor and tints its first
 * corner by the three flag bits. It is still an asm boundary in this payload, so
 * it keeps its address anchor. */
void func_801F36C0(u8* position, s32 angle, s32 flags);

/* Effect entry seeder at 0x801F3E90: it copies the work object's 0x34/0x38/0x3C
 * position words into the 0x2C-byte entry handed in $a0, seeds that entry's
 * 0x10..0x1C corner offset halfwords from the shared angular pair at +0x18 and
 * -0x18, rotates both offset vectors through a random Euler matrix, arms the
 * entry scale halfword at +0x28 and clears its phase halfwords. */
void seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area17213_801F3E90(
    u8* entry);

void clearWorkTableAt800E4800World04Area17213_801F38BC(void);

/* @source 0x801F30F8
 * @behavior Overlay progress step handler, entry 7 of the state byte 1 handler
 *           run at 0x801F507C and entry 2 of the run at 0x801F5090. It emits
 *           the work quad through 0x801F36C0 from the work position words at
 *           +0x34 and the angle halfword at +0x2E with flag mask 7, and then,
 *           only while the shared scenario progress byte reads 0x0A, clears the
 *           lead byte of the eight 0x18-byte records at 0x800E4800, re-seeds
 *           and draws all 0x20 entries of the 0x2C-byte-stride effect table at
 *           0x800E48C0 through 0x801F3E90 between a PushMatrix/PopMatrix pair,
 *           clears the work frame counter byte at +0x06, sets the level byte at
 *           +0x5D to 8 and the countdown byte at +0x09 to 0x78, and advances the
 *           reloaded work state byte at +0x01. Takes no arguments and returns
 *           nothing.
 *
 * Evidence from the original bytes at 0x801F30F8: addiu $sp,$sp,-0x20 with sw
 * $ra,0x18, $s1,0x14 and $s0,0x10, the `lui 0x1F80` plus lw 0x44 pair before the
 * frame, addiu $a2,$zero,0x7, lh $a1,0x2E($v0), jal 0x801F36C0 with addiu
 * $a0,$v0,0x34 in its delay slot, then the lui/%hi and folded lbu/%lo of
 * 0x80146864 against addiu $v0,$zero,0xA with bne to the epilogue. The guarded
 * block starts with jal 0x801F38BC, jal PushMatrix and addu $s0,$zero,$zero,
 * walks jal 0x801F3E90 with addu $a0,$s1,$zero and the tail addiu $s0,1,
 * andi $v0,$s0,0xFF, sltiu $v0,$v0,0x20 and bnez with addiu $s1,$s1,0x2C in its
 * delay slot, and calls PopMatrix. The four store groups that follow each carry
 * their own `lui 0x1F80` plus lw 0x44 pair: sb $zero,0x6($v0), sb 8 to +0x5D,
 * sb 0x78 to +0x9, and finally lbu $v0,0x1($v1), addiu $v0,$v0,1 and
 * sb $v0,0x1($v1). The base 0x800E48C0 is 0x800E4800 + 8 * 0x18, which is the
 * eight-record block its lead-byte clear covers.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkQuadReseedEffectEntriesAtProgress0aThenArmCountdownAndAdvanceStateWorld04Area17213_801F30F8(
    void) {
  u8* work;
  u8* entry;
  u8 index;

  work = D_1F800044;
  func_801F36C0(work + 0x34, *(s16*)(work + 0x2e), 7);
  if (g_ScenarioProgress == 0xa) {
    clearWorkTableAt800E4800World04Area17213_801F38BC();
    PushMatrix();
    entry = PSX_PTR(u8, 0x800E48C0u);
    index = 0u;
    do {
      seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area17213_801F3E90(
          entry);
      entry += 0x2cu;
      index += 1u;
    } while (index < 0x20u);
    PopMatrix();
    D_1F800044[6] = 0;
    D_1F800044[0x5d] = 8;
    D_1F800044[9] = 0x78;
    D_1F800044[1]++;
  }
}
