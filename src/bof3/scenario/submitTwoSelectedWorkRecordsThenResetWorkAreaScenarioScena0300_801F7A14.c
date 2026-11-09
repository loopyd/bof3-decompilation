#include "bof3/bof3.h"

extern u8 *D_1F800044;
extern u8 workRecordBytesScenarioScena0300_80146888[]; /* @source 0x80146888 @kind data */

void func_80196718(void *arg0);
void func_80196070(void);

/* @source 0x801F7A14
 * @behavior Entry 2 of the overlay-local work-byte-1 handler table at
 * 0x801FCF50, which dispatchWorkByte1HandlerScenarioScena0300_801F7508 selects
 * with byte 1 of the work object published at scratchpad pointer slot
 * 0x1F800044 and calls with no arguments. Reads that work pointer, submits the
 * 0x98-byte record at 0x80146888 selected by work byte 3 through func_80196718,
 * reloads the work pointer and submits the record selected by work byte 4 the
 * same way, then calls func_80196070, which clears the scratchpad work-area
 * header flags_00, unk_01, flags_02 and pad_03[0..1]. It writes no other state.
 * Takes no arguments and returns no value; the 0x18-byte frame only keeps $ra
 * and $s0 across the three calls, and each index half re-reads the scratchpad
 * pointer cell because the store through the loaded pointer may alias it.
 * The body is the one the twin at 0x801F7334 (entry 2 of the handler table at
 * 0x801FCF24) ships as its own source: the two 0x88-byte payload spans compared
 * equal over all 136 bytes, and every absolute word they carry names the same
 * out-of-payload cell or callee, so the seeded statement order is that body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitTwoSelectedWorkRecordsThenResetWorkAreaScenarioScena0300_801F7A14(void) {
  func_80196718(&workRecordBytesScenarioScena0300_80146888[D_1F800044[3] * 0x98]);
  func_80196718(&workRecordBytesScenarioScena0300_80146888[D_1F800044[4] * 0x98]);
  func_80196070();
}
