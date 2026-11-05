#include "bof3/bof3.h"

extern u8 *D_1F800044;
extern u8 countdownFlagByteScenarioScena0300_80145E90[]; /* @source 0x80145E90 @kind data */
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */

/* @source 0x801F7260
 * @behavior Handles work-byte-1 state 1 through the table at 0x801FCF24.
 * Reads the work pointer from scratchpad cell 0x1F800044. While work byte 9
 * is nonzero, sets bit 0x40 of the byte at 0x80145E90 and decrements work
 * byte 9. Otherwise clears that bit, clears byte 0 of each 0x98-byte record
 * at 0x80146888 selected by work bytes 3 and 4, and sets work byte 1 to 2.
 * Reloads the work pointer after each indexed record store, preserving the
 * original alias-sensitive accesses. Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void tickCountdownAndClearTwoRecordBytesScenarioScena0300_801F7260(void) {
  u8 *work;
  u32 address_or_index;

  work = D_1F800044;
  if (work[9] != 0) {
    countdownFlagByteScenarioScena0300_80145E90[0] |= 0x40;
    work[9]--;
  } else {
    countdownFlagByteScenarioScena0300_80145E90[0] &= ~0x40;
    /* The PSX address fits u32. Reuse it only after the byte store; the
     * second value is the freshly loaded unsigned record index, 0..255. */
    address_or_index = (u32)&workRecordBytesScenarioScena0300_80146888[work[3] * 0x98];
    *(u8 *)address_or_index = 0;
    address_or_index = D_1F800044[4];
    workRecordBytesScenarioScena0300_80146888[address_or_index * 0x98] = 0;
    D_1F800044[1] = 2;
  }
}
