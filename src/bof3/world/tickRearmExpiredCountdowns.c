#include "bof3/bof3.h"

void renderClampedVerticalBar(u8* rec);
void stateRandomAroundObject(
    u8* record);

/* @source 0x801F39D8
 * @behavior Per-frame pass over the 0x80 records of the 0xE-byte-stride record
 *           table at 0x800E4800 (the main-RAM work area this overlay shares
 *           with the world and scenario overlays), walked with an unsigned byte
 *           counter in a bottom-tested loop. Every record whose +0xC countdown
 *           halfword still runs (read as a signed halfword, `lh`/`beqz`) is
 *           drawn as a clamped vertical bar through
 *           renderClampedVerticalBar with the
 *           record base in $a0, and then advanced: the +0x2 halfword drops by
 *           the +0x6 halfword, the +0x6 halfword grows by the countdown's low
 *           bit, the +0x4 halfword grows by 1 and the countdown itself drops by
 *           1. When that decrement reaches zero the record's position is
 *           re-seeded and its countdown re-armed to 0x20 through
 *           stateRandomAroundObject,
 *           again with the record base in $a0. The pass takes no arguments,
 *           returns nothing and keeps a 0x20-byte frame for $s0-$s2 and $ra.
 *           Its only in-payload caller is the state handler func_801F2F5C (jal
 *           at 0x801F2FB4), which runs it once per frame after its own draw
 *           helper, so this is the overlay's work-record tick.
 *
 * Name evidence: the in-payload call site gives the per-frame tick role, and
 *           the body's own stores give the rest - the draw callee for the
 *           rendering half, the countdown test and decrement with the re-seed
 *           helper on the reaching-zero path for the re-arm. The 0x80-entry,
 *           0xE-stride table and the masked byte counter are the ones the exact
 *           siblings stateWorkRecordCountdowns
 *           and queryFreeRecordCountdown
 *           walk, and the table/stride/field vocabulary follows them.
 *
 * Shape evidence: the loop is the exact siblings' bottom-tested masked byte
 *           counter (`andi` then `sltiu` against 0x80) with the pointer advance
 *           ahead of the counter increment, and the countdown test is the `lh`
 *           form the free-record scan documents for the same field. The clone
 *           lever is declined on measured evidence: a scan of all 795 payloads
 *           under out/binaries (792 emi plus 3 exe) for this 0xC4-byte span
 *           found only the span's own offset, and it stays a miss with both
 *           `jal` immediates masked, so no sibling seeds these statements
 *           either; a family member cannot be byte-identical anyway because the
 *           body encodes its own two payload-local callees.
 *
 * Representation note: the decremented countdown is a signed halfword local
 *           stored back into the unsigned lvalue. The first gate held the
 *           lvalue test and measured 48/49 at 196 bytes, first difference at
 *           offset 0x7C where the original narrows the decremented value with
 *           `sll v1,v1,0x10` and the candidate masked it with `andi
 *           v1,v1,0xFFFF`. Casting the tested lvalue to `s16` did not move that
 *           instruction (48/49); a `u16` local carrying the decrement then
 *           testing `(s16)countdown` measured 34/49 and 192 bytes; the `s16`
 *           local tested directly is what emits the original's narrowing and
 *           reached 49/49 at the unchanged 196 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickRearmExpiredCountdowns(
    void) {
  u8* work;
  u8 count;
  s16 countdown;

  work = PSX_PTR(u8, 0x800E4800u);
  count = 0u;

  do {
    if (FIELD_REF(s16, work, 0xCu) != 0) {
      renderClampedVerticalBar(work);
      FIELD_REF(u16, work, 0x2u) -= FIELD_REF(u16, work, 0x6u);
      FIELD_REF(u16, work, 0x6u) += FIELD_REF(u16, work, 0xCu) & 1u;
      FIELD_REF(u16, work, 0x4u) += 1u;
      countdown = FIELD_REF(u16, work, 0xCu) - 1;
      FIELD_REF(u16, work, 0xCu) = countdown;
      if (countdown == 0) {
        stateRandomAroundObject(
            work);
      }
    }
    work += 0xEu;
    count += 1u;
  } while (count < 0x80u);
}
