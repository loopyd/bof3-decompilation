#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */

/* @source 0x801EFCF4
 * @behavior Calls 0x800AF764, increments work byte 9, and subtracts
 *           (the previous byte 9 minus eight) divided by two, rounded toward
 *           zero, from work halfword 0x30. Advances work byte 2 when the
 *           incremented byte 9 equals 28, re-reading the work pointer cell.
 * @status partial
 * @match 93.75
 * @residual The two original byte-9 loads coalesce into one load and a nop.
 */
void callAF764IncrementWorkByte9SubtractHalfPreviousByte9Minus8FromHalfword30ThenAdvanceWorkByte2At28BmagicMagic10303_801EFCF4(void) {
  u8 *work;
  u8 counter;
  s32 delta;
  u16 value;

  func_800AF764();
  work = g_battle_work;
  counter = work[9] + 1;
  delta = ((s32)work[9] - 8) / 2;
  value = FIELD_REF(u16, work, 0x30);
  work[9] = counter;
  FIELD_REF(u16, work, 0x30) = value - delta;
  if (counter == 28) {
    g_battle_work[2]++;
  }
}
