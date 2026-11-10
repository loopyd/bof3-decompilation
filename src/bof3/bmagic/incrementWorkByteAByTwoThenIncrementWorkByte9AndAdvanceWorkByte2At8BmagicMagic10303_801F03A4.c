#include "bof3/bof3.h"

/* Stores through the work pointer can alias its scratchpad pointer cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F03A4
 * @behavior Adds two to work byte 0xA, re-reads the pointer cell at
 *           0x1F800044, and increments work byte 9. When that truncated
 *           byte equals eight, re-reads the cell and increments work byte 2.
 *           Leaf with no arguments, return value or stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAByTwoThenIncrementWorkByte9AndAdvanceWorkByte2At8BmagicMagic10303_801F03A4(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  work[0xA] += 2;
  counter = g_battle_work[9] + 1;
  g_battle_work[9] = counter;
  if (counter == 8) {
    g_battle_work[2]++;
  }
}
