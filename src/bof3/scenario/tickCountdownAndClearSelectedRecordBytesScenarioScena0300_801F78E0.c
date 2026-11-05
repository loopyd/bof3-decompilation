#include "bof3/bof3.h"

extern u8 *D_1F800044;
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */

/* @source 0x801F78E0
 * @behavior Handles work-byte-1 state 1 through the table at 0x801FCF50.
 * While work byte 9 is nonzero, sets bit 0x40 of the lead byte of the
 * 0x98-byte record selected by work byte 11, then decrements work byte 9.
 * At zero, clears that bit, clears the lead bytes of the records selected
 * by work bytes 3 and 4, and sets work byte 1 to 2. Reloads the scratchpad
 * work pointer after each record store to preserve alias-sensitive accesses.
 * Takes no arguments and returns no value.
 * @status partial
 * @match 84.42
 * @residual Selected-record offset and value use swapped v0/v1 registers in both arms.
 */
void tickCountdownAndClearSelectedRecordBytesScenarioScena0300_801F78E0(void) {
  u8 *work;
  u32 address_or_index;

  work = D_1F800044;
  if (work[9] != 0) {
    address_or_index = work[11] * 0x98;
    workRecordBytesScenarioScena0300_80146888[address_or_index] = workRecordBytesScenarioScena0300_80146888[address_or_index] | 0x40;
    D_1F800044[9]--;
  } else {
    address_or_index = work[11] * 0x98;
    workRecordBytesScenarioScena0300_80146888[address_or_index] = workRecordBytesScenarioScena0300_80146888[address_or_index] & ~0x40;
    /* Each previous value is consumed before reuse. PSX pointers fit u32;
     * the unsigned byte record index times 0x98 cannot overflow. */
    address_or_index = (u32)&workRecordBytesScenarioScena0300_80146888[D_1F800044[3] * 0x98];
    *(u8 *)address_or_index = 0;
    address_or_index = D_1F800044[4];
    workRecordBytesScenarioScena0300_80146888[address_or_index * 0x98] = 0;
    D_1F800044[1] = 2;
  }
}
