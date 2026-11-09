#include "bof3/bof3.h"

extern u8 *D_1F800044;
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */

void func_80196718(void *arg0);
void func_80196070(void);

/* @source 0x801F7334
 * @behavior Handles work-byte-1 state 2 through the overlay-local table at
 * 0x801FCF24. Reads the work pointer from scratchpad cell 0x1F800044, submits
 * the 0x98-byte record at 0x80146888 selected by work byte 3 through
 * func_80196718, reloads the work pointer and submits the record selected by
 * work byte 4 the same way, then calls func_80196070, which clears the
 * scratchpad work-area header flags_00, unk_01, flags_02 and pad_03[0..1]. It
 * writes no other state. Takes no arguments and returns no value; the
 * 0x18-byte frame only keeps $ra and $s0 across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitTwoSelectedWorkRecordsThenResetWorkAreaScenarioScena0300_801F7334(void) {
  func_80196718(&workRecordBytesScenarioScena0300_80146888[D_1F800044[3] * 0x98]);
  func_80196718(&workRecordBytesScenarioScena0300_80146888[D_1F800044[4] * 0x98]);
  func_80196070();
}
