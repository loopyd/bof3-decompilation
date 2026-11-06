#include "bof3/bof3.h"

/* Current AREA108 work-record cursor. The overlay seeds this cell with the
 * 0x800E4800 work-table base and walks it by the 0x28-byte record stride
 * (func_801F2FC8 and func_801F3034 do that for the 16-record table), so the
 * cell always points at the record the per-frame handlers update. It is the
 * payload's last word and is zero on disc; the address is bound by this
 * target's generated undefined_syms_auto.txt, so no map row is needed. */
extern u8* workRecordCursor63ecWorld02Area10814; /* @source 0x801F63EC @kind data */

/* @source 0x801F3198
 * @behavior Overlay per-frame step handler: reads the work-record cursor cell
 * at 0x801F63EC and advances the current record's three word positions at
 * +0x04, +0x08 and +0x0C by the deltas at +0x14, +0x18 and +0x1C, decrements
 * its byte countdowns at +0x03 and +0x02, and clears the record's active byte
 * at +0x00 when the +0x02 countdown reaches zero. The active byte is what
 * func_801F3034 tests before it dispatches the record's +0x01 handler, so the
 * clear retires the record. The cell is re-read for the +0x02 group and again
 * before the active-byte clear: the original loads it three times because the
 * intervening stores through the old pointer may alias the cell. The record is
 * addressed through a word-typed view with the countdown byte reached through a
 * byte pointer derived from it; that split is the form that reproduces the
 * original's word copies and its byte decrement. Takes no arguments and returns
 * nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkDeltasAndExpireCountdownWorld02Area10814_801F3198(void) {
  s32* words;
  u8 countdown;

  words = (s32*)workRecordCursor63ecWorld02Area10814;
  words[1] += words[5];
  words[2] += words[6];
  words[3] += words[7];
  ((u8*)words)[3]--;
  countdown = (u8)(workRecordCursor63ecWorld02Area10814[2] - 1);
  workRecordCursor63ecWorld02Area10814[2] = countdown;
  if (countdown == 0) {
    workRecordCursor63ecWorld02Area10814[0] = 0;
  }
}
