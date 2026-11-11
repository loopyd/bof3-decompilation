#include "bof3/bof3.h"

/* @source 0x801F3568
 * @behavior Overlay step gate over the scratchpad work record published at
 *           0x1F800044: reads the record byte at +9 once and tests it
 *           unsigned against 8. While that byte is still below 8 the gate
 *           only increments it. Once it has reached 8 or more the gate first
 *           increments the record state byte at +2, re-reads the record
 *           pointer from the scratchpad pointer cell, and then increments the
 *           byte at +9 through the re-read pointer. Both paths close with the
 *           same lbu/addiu/sb on +9, and that store fills the jr $ra delay
 *           slot. Takes no arguments, returns nothing, reserves no frame and
 *           keeps no saved register.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceState2WhenWorkByte9Reaches8ThenAdvanceWorkByte9World04Area16613_801F3568(
    void) {
  u8* work;

  work = SPAD_ADDR(u8, 0x44u);
  work = *(u8**)work;
  if (work[9] >= 8) {
    work[2]++;
    work = *(u8**)SPAD_ADDR(u8, 0x44u);
  }
  work[9]++;
}
