#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F2C48
 * @behavior Entry 0 of the state byte 1 handler run dispatched through the
 *           pointer table at 0x801F507C, so the work state byte at +1 of the
 *           scratch work record reached through the pointer cell at 0x1F800044
 *           reads 0 when it runs. It writes 0 into the signed level bytes 0x5F,
 *           0x5E and 0x5D of that record through one reload of the cell, then
 *           reloads the cell to seed the record countdown byte +9 with 0x5A, the
 *           value its successor 0x801F2C84 counts down while it derives the level
 *           triple, and reloads the cell once more to write 1 into the state byte
 *           +1, which hands the record to that successor. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateScratchLevelTriple(void) {
  u8* record;

  record = D_1F800044;
  record[0x5f] = 0;
  record[0x5e] = 0;
  record[0x5d] = 0;
  D_1F800044[9] = 0x5a;
  D_1F800044[1] = 1;
}
