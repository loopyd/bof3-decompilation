#include "bof3/bof3.h"

extern u8 D_801F6514;

/* @source 0x801F4FEC
 * @behavior Frame handler, entry 2 of the three-word handler row based at
 *           0x801F5FBC: on its first pass it sets the one-shot latch byte at
 *           0x801F6514 to 1 when that byte still reads zero, then decrements the
 *           countdown byte at +0x02, adds 0x400 to the record word at +0x08, adds
 *           that updated word into the record word at +0x10, subtracts 8 from the
 *           record byte at +0x03, and clears the record byte at +0x00 when the
 *           countdown reaches zero. The record pointer is the only argument and
 *           the function returns nothing; it is a leaf with no frame.
 *
 *           Dispatch evidence: the payload word 0x801F4FEC is the third entry of
 *           the three-word row at 0x801F5FBC (0x801F4F60, 0x801F4FB8, 0x801F4FEC;
 *           the script blob begins at 0x801F5FC8), read directly from this
 *           target's payload. func_801F4E7C, still an asm span here, walks the 16
 *           records of stride 0x1C at 0x800E4800, skips the records whose byte at
 *           +0x00 reads zero, scales each record byte at +0x01 by four onto that
 *           row base, jalrs the selected entry with the record pointer in $a0, and
 *           then calls func_801F4BC0 with the same record. No jal immediate names
 *           this address, so the record pointer role comes from that indirect call
 *           site and stays structural.
 *
 *           Latch evidence: the exact reset handler
 *           resetWorkTableAt800E4800AndRearmOneShotLatchesWorld02Area09813_801F4D44
 *           zeroes 0x801F6510 and 0x801F6514 and reads both as one-shot flags that
 *           entry 0 of this row and this entry set to 1 on their first pass, which
 *           is the naming evidence for the latch role here.
 *
 *           Shape evidence: the countdown half follows the statement order and
 *           operand forms of the two exact handler lifts of this target,
 *           countdownReloadAdvanceHandlerAndSetOneShotLatchWorld02Area09813_801F4F60
 *           (countdown decrement, unsigned mask, latch set on the first pass) and
 *           countdownReload16ThenAdvanceHandlerWorld02Area09813_801F4FB8, both of
 *           which index the same record with byte offsets; the two word steps at
 *           +0x08 and +0x10 are this span's own, written from its original bytes.
 *
 *           Representation notes: the original reads the record through a register
 *           copy of the incoming pointer (move a2,a0 in the delay slot of the latch
 *           test) and keeps the decremented countdown in a register, masking it
 *           with andi 0xFF for the expiry test. Using the argument in place, with
 *           the expiry test re-reading +0x02, measured 8/25 instructions with the
 *           first difference at +0x0C (latch block one instruction short, no
 *           pointer copy). Adding only the pointer copy while the test still
 *           re-read +0x02 also measured 8/25 with the same first difference. The
 *           pointer copy plus an unsigned-byte temporary for the decremented
 *           countdown measured 25/25 instructions over 100 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkWord8IntoWord10ReleaseRecordOnCountdownZeroAndSetOneShotLatchWorld02Area09813_801F4FEC(
    u8* arg0) {
  u8* record = arg0;
  u8 remaining;

  if (D_801F6514 == 0) {
    D_801F6514 = 1;
  }
  remaining = --record[2];
  *(u32*)(record + 0x8) += 0x400;
  *(u32*)(record + 0x10) += *(u32*)(record + 0x8);
  record[3] -= 8;
  if (remaining == 0) {
    record[0] = 0;
  }
}
