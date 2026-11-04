#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F3034
 * @behavior Writes 8 to byte +9 of the scratchpad work record at pointer
 * slot 0x1F800044, reloads that pointer, clears halfword +0x2E of the
 * original record, and increments state byte +1 of the reloaded record.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchWorkByte9ClearHalf2eAdvanceStateWorld04Area17213_801F3034(void) {
  u8* work;
  u8* next;

  work = D_1F800044;
  work[9] = 8;
  next = D_1F800044;
  *(u16*)(work + 0x2e) = 0;
  next[1]++;
}
