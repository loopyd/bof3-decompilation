#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */

/* @source 0x801EFC70
 * @behavior Calls 0x800AF764, adds two to work byte 0xA, re-reads
 *           the pointer cell at 0x1F800044, and increments work byte 9.
 *           When that truncated byte equals eight, re-reads the cell
 *           and increments work byte 2. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAF764IncrementWorkByteAByTwoThenIncrementWorkByte9AndAdvanceWorkByte2At8BmagicMagic10303_801EFC70(void) {
  u8 *work;
  u8 counter;

  func_800AF764();
  work = g_battle_work;
  work[0xA] += 2;
  counter = g_battle_work[9] + 1;
  g_battle_work[9] = counter;
  if (counter == 8) {
    g_battle_work[2]++;
  }
}
