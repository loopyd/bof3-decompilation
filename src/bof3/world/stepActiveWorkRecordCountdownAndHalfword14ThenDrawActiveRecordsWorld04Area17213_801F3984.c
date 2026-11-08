#include "bof3/bof3.h"

/* Shared projection setup that fills the 0x20-byte stack scratch the frame
 * passes of this payload read; func_801F3FAC, func_801F34D8, func_801F35D8 and
 * func_801F36C0 call it from the same generated map row of this target. */
void func_801AFE18(void* arg0);

/* 0x801F3A38 - this overlay's per-record projection and draw pass, fed the
 * entry base in $a0 by the sweep below. It stays an asm boundary in this lane,
 * so it keeps its address anchor. */
void func_801F3A38(u8* record);

/* @source 0x801F3984
 * @behavior Overlay per-frame record pass: it fills the 0x20-byte projection
 *           scratch through func_801AFE18 and then walks the 8 entries of the
 *           0x18-byte-stride work table at 0x800E4800 (the main-RAM work area
 *           this overlay shares with the world, scenario and battle overlays)
 *           with an unsigned byte counter in a bottom-tested loop. For every
 *           entry whose claim byte at +0x00 reads non-zero it marks the frame
 *           active, steps that entry's countdown byte at +0x02 down by one and
 *           its halfword at +0x14 down by 0x0C, clears the claim byte on the
 *           frame the decremented countdown reaches zero, and then calls the
 *           per-record projection and draw pass 0x801F3A38 with the entry base
 *           in $a0, which reads that entry's +0x04/+0x08/+0x0C position words
 *           and its +0x14 halfword. It returns 1 on every frame in which at
 *           least one entry was claimed and 0 when the whole table is free;
 *           both of its in-image call sites, the jal at 0x801F3220 in
 *           func_801F31D0 and the jal at 0x801F3320 in the exact handler
 *           0x801F32E8, discard the result, and no word of this payload names
 *           the address, so those two calls are the only references to it.
 *           Takes no arguments; the 0x48-byte frame holds the 0x20-byte scratch
 *           at +0x10 and the saved $ra with $s0-$s3 of the sweep from +0x30.
 *
 * Evidence from the original bytes at 0x801F3984: addiu $sp,$sp,-0x48 with
 * sw $s3,0x3C, $ra,0x40, $s2,0x38, $s1,0x34 and $s0,0x30, the $s3 zeroing
 * before the jal to 0x801AFE18 and addiu $a0,$sp,0x10 in its argument slot, then
 * the lui/ori pairs for 0x800E4800 and 0x800E4802 around the $s2 counter
 * zeroing. The body is lbu $v0,0x0($s1) with nop and beqz to the loop join,
 * addiu $s3,$zero,0x1, lbu $v1,0x0($s0) and lhu $v0,0x12($s0), addiu $v1,$v1,
 * -0x1 and addiu $v0,$v0,-0xC, sb $v1,0x0($s0), andi $v1,$v1,0xFF and bnez to
 * the call with sh $v0,0x12($s0) in its delay slot, sb $zero,0x0($s1) on the
 * fall-through, then jal 0x801F3A38 with addu $a0,$s1,$zero. The tail adds 0x18
 * to $s0, then the counter, then andi/sltiu 8 with the taken branch and
 * addiu $s1,0x18 in its delay slot, and the epilogue restores the frame and
 * returns $s3 through addu $v0,$s3,$zero.
 *
 * Field evidence for the stepped pair comes from the claim helper 0x801F38E8 in
 * this payload: it claims a free entry by storing 1 at +0x00, 0 at +0x01, 0x20
 * at +0x02, the work object's 0x34/0x38/0x3C words at +0x04/+0x08/+0x0C and
 * 0x180 at +0x14. A 0x20-frame countdown and a 0x180 halfword stepped by 0x0C
 * expire on the same frame, which is why one body steps both and releases the
 * entry when the countdown runs out. The +0x14 halfword carries no name in this
 * tree, so the lift keeps it address-anchored as the halfword at 0x14.
 *
 * Shape evidence: the 0xB4-byte body has no byte-identical twin anywhere in
 * out/binaries - a whole-image search for its 180 bytes matches this payload
 * alone - so the clone lever is declined. func_801F5308 of emi/world02/area098/13
 * and func_801F3E7C of emi/world04/area188/13 carry the same sweep instruction
 * for instruction and differ only in their decay immediate (0x10 there, 0x0C
 * here), their block labels and their own jal immediate; both are still asm
 * boundaries, so they supply shape only. The walk itself follows the exact
 * sibling updateWorkTableAndReportActiveWorld02Area08213_801F3AAC, which returns
 * the same frame-active flag over the same shared table.
 *
 * Matching note: the entry offset pointer has to be formed inside the loop.
 * Seeding it beside the table base and the counter measured 22/45 at 196 bytes,
 * where the folded halfword address became a third table base and pushed the
 * frame-active flag into $s4; the same body with the offset pointer initialized
 * at the top of the loop, so that the loop pass derives it from the table base,
 * measured 45/45 at the unchanged 180 bytes. With the offset pointer in place
 * the halfword step also has to come before the countdown store: the countdown
 * store first measured 41/45 at 180 bytes, where the loop pass derived the
 * halfword address as the base instead of the entry offset. Both orderings
 * leave every value and width unchanged, so read the original's base register
 * before choosing them.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 stepActiveWorkRecordCountdownAndHalfword14ThenDrawActiveRecordsWorld04Area17213_801F3984(
    void) {
  u8 scratch[0x20];
  u8* record;
  u8* field;
  u8 index;
  u8 countdown;
  s32 active;

  active = 0;
  func_801AFE18(scratch);
  record = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  do {
    field = record + 2;
    if (*record != 0) {
      active = 1;
      countdown = field[0] - 1;
      FIELD_REF(u16, field, 0x12) -= 0x0Cu;
      field[0] = countdown;
      if (countdown == 0) {
        *record = 0;
      }
      func_801F3A38(record);
    }
    record += 0x18u;
    index += 1u;
  } while (index < 8u);

  return active;
}
