#include "bof3/bof3.h"

/* Current AREA108 work-record cursor. The overlay seeds this cell with the
 * 0x800E4800 work-table base and walks it by the 0x28-byte record stride
 * (stateSixteenWorkRecords and func_801F3034 do that
 * for the 16-record table), so the cell always points at the record the
 * per-frame handlers update. It is the payload's last word and is zero on disc;
 * the address is bound by this target's generated undefined_syms_auto.txt, so no
 * map row is needed. */
extern u8* workRecordCursor; /* @source 0x801F63EC @kind data */

/* @source 0x801F30FC
 * @behavior Overlay per-frame step handler, entry 3 of the state-byte-1
 * handler table at 0x801F56E8: reads the work-record cursor cell at 0x801F63EC
 * and advances the current record's three word positions at +0x04, +0x08 and
 * +0x0C by twice its velocity words at +0x14, +0x18 and +0x1C, decrements its
 * +0x02 byte timer and advances its +0x24 halfword phase by 0x80. When that
 * timer reaches zero it reseeds the +0x02 timer with 0x40 and advances the
 * record dispatch byte at +0x01 by one, so the record moves to the next state.
 * The cell is re-read before each statement of the timer-expiry arm because the
 * intervening stores through the old pointer may alias it. The record is
 * addressed through a word-typed view with the timer reached through a byte
 * pointer derived from it and the phase through a halfword view. The statement
 * order comes from the same-shape member advanceWorkStateWithDoubleVelocity in
 * emi/world00/area024/14, whose span differs from this one only in the three
 * %lo words naming its own cursor cell.
 * Takes no arguments and returns nothing; leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkDoubleVelocity(void) {
  s32* words;
  u8 timer;

  words = (s32*)workRecordCursor;
  timer = ((u8*)words)[2] - 1;
  words[1] += words[5] * 2;
  words[2] += words[6] * 2;
  words[3] += words[7] * 2;
  ((u8*)words)[2] = timer;
  ((u16*)words)[0x12] += 0x80;
  if (timer == 0) {
    ((u8*)workRecordCursor)[2] = 0x40;
    workRecordCursor[1]++;
  }
}
