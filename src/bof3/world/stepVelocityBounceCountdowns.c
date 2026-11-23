#include "bof3/bof3.h"

/* 0x801F32A4 - this overlay's per-entry drawer. It takes the record base
 * pointer, builds a texture page through GetTPage, and projects and emits a
 * semi-transparent prim from the record words at +0x14 and +0x18, so its only
 * argument is the base pointer the sweep below already walks. The span is still
 * an asm row in this target. */
void func_801F32A4(u8* record); /* @source 0x801F32A4 */

/* @source 0x801F3368
 * @behavior Per-frame sweep of the 0x80 entries of the 0x24-byte-stride effect
 *   table at 0x800E5C00. It skips every entry whose lead byte is zero; for a
 *   live entry it adds the three step words at record +0x04/+0x08/+0x0C to the
 *   position words at +0x14/+0x18/+0x1C, lowers the third step word by 0x40000,
 *   and when the third position word reads below -0x2000000 it clamps that word
 *   to -0x2000000 and negates the third step word, so the entry bounces off that
 *   floor. It then lowers the counter at +0x03 by 2, decrements the counter at
 *   +0x02 and clears the in-use flag once that counter reaches zero, draws the
 *   entry through func_801F32A4 with the record base pointer and sets its result
 *   to 1. It takes no arguments and returns 1 when at least one entry was live
 *   and 0 otherwise; its only in-payload caller is the step handler at
 *   0x801F2DA4, whose jal at 0x801F2E08 tests that result's low byte before it
 *   advances the state byte at 0x1F800044+1. It calls the main-executable
 *   rand() nowhere, which is what separates it from the 0x800E4800 sweep below.
 *
 * Name evidence: 0x800E5C00 is the work table this target's own init helper
 *   clearWorkTableAt800E5C00World04Area19713_801F3474 empties with the same
 *   0x80-entry, 0x24-byte stride, so the name keeps that module vocabulary. The
 *   sibling sweep at 0x801F2FC4 in this same target walks the 0x800E4800 table
 *   with the same loop skeleton, the same -0x40000 step decrement and the same
 *   0xFE000000 floor clamp, and the world03 area120 sweep
 *   advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0 establishes
 *   the pattern this body follows: test the lead byte, step the record words,
 *   expire the counter into the lead byte, then draw the live record through a
 *   per-record drawer and report that one was handled. The counters and the two
 *   triples have no reviewed role beyond this arithmetic, so every field stays
 *   offset-anchored. The only symbol naming 0x801F3368 is the raw func_ row.
 *
 * Shape evidence: one payload scan over all 398 extracted binaries under
 *   out/binaries found this 268-byte body once, at file offset 0x768 of this
 *   target, so no byte-identical sibling exists anywhere and the clone lever is
 *   declined. A second scan that compares opcode and register fields alone over
 *   the leading 24 instructions found one other member, the same-target sweep at
 *   0x801F2FC4, whose leading words differ in 9 immediates (the two base addends
 *   and the record field offsets) and whose full 0x120-byte body carries the
 *   same statement order with its own 0x28 stride, an extra rand() sample in its
 *   counter block and its field triple four bytes higher. That disassembly is
 *   original bytes, so it supplied the statement order, the operand forms, the
 *   declaration style and the -0x40000 / -0x2000000 forms; the base addend, the
 *   stride, the 0x80 count, the field offsets and the +0x02 and +0x03 counter
 *   steps all come from this span's bytes.
 *
 *   The walk keeps one base pointer and byte offsets because the original reads
 *   every field from the 0x800E5C00 entry base: its prologue materialises a
 *   second 0x800E5C02 addend that loop.c derives for the counters and the word
 *   triples, and the words then sit at displacements 0x02/0x06/0x0A and
 *   0x12/0x16/0x1A from that derived base, exactly as this source produces. A
 *   struct view cannot reproduce them, because the s32 members of a struct whose
 *   first two bytes are the counters align to 0x04. Statement order matters
 *   here: with the +0x02 counter statement before the +0x03 one, loop.c derives
 *   the 0x800E5C03 addend instead and the body grows by three instructions.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 stepVelocityBounceCountdowns(
    void) {
  u8* record;
  u16 i;
  s32 handled;

  handled = 0;
  record = PSX_PTR(u8, 0x800E5C00u);
  i = 0u;
  do {
    if (record[0] != 0) {
      *(s32*)(record + 0x14) = *(s32*)(record + 0x14) + *(s32*)(record + 0x04);
      *(s32*)(record + 0x18) = *(s32*)(record + 0x18) + *(s32*)(record + 0x08);
      *(s32*)(record + 0x1c) = *(s32*)(record + 0x1c) + *(s32*)(record + 0x0c);
      *(s32*)(record + 0x0c) = *(s32*)(record + 0x0c) - 0x40000;
      if (*(s32*)(record + 0x1c) < -0x2000000) {
        *(s32*)(record + 0x1c) = -0x2000000;
        *(s32*)(record + 0x0c) = -*(s32*)(record + 0x0c);
      }
      record[3] = record[3] - 2;
      if (--record[2] == 0) {
        record[0] = 0;
      }
      func_801F32A4(record);
      handled = 1;
    }
    record += 0x24;
    i += 1u;
  } while (i < 0x80u);

  return handled;
}
