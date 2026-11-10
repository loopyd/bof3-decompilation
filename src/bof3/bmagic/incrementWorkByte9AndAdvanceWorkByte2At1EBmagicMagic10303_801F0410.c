#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The store through the loaded pointer
 * can alias this cell, so the second access re-reads it without volatility. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0410
 * @behavior Increments work byte 9 through the pointer cell at 0x1F800044,
 *           tests the truncated byte against 0x1E, and only on equality
 *           re-reads the pointer cell and increments work byte 2. Leaf with
 *           no arguments, return value or stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndAdvanceWorkByte2At1EBmagicMagic10303_801F0410(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 0x1E) {
    g_battle_work[2]++;
  }
}
