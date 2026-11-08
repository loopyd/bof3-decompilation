#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after the countdown store and again before each of
 * the three later record accesses, so the source keeps the named-cell form and
 * evaluates it again at every access instead of caching the pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Work-sequence selector of the shared EXE image, outside this payload. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt,
 * and the exact sibling lifts of this overlay declare the same prototype.
 */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CEB10
 * @behavior Work byte 2 == 1 step of this overlay handler run: the word at
 *           0x801D07B4 is entry 1 of the run based at 0x801D07A4, which the
 *           byte-2 dispatcher at 0x801CEA00 indexes by work byte 0x02, and the
 *           preceding step at 0x801CEA44 stores the chosen candidate route index
 *           in byte 0x03 together with the step sign in byte 0x0B and rearms the
 *           countdown byte 0x09 to 2. This body takes the work record published
 *           at the scratchpad pointer cell 0x1F800044 and compares the current
 *           route index byte 0x08 with that stored route byte 0x03. When the two
 *           agree the route has been reached, so it selects the shared work
 *           sequence through func_8014D8D4 with 0x40 for route 3 and 0x41 for
 *           route 5, then increments the step counter byte 0x02, which advances
 *           the dispatch to entry 2 of the same run at 0x801CEC0C. Otherwise it
 *           decrements the countdown byte 0x09 and returns without advancing;
 *           only when that reloaded byte reaches zero does it store 2 back into
 *           the countdown, add the step sign byte 0x0B into the route index byte
 *           0x08, mask the route index to the low three bits of the eight-entry
 *           direction ring and select the shared work sequence named by that new
 *           route index. The original re-reads the pointer cell after the
 *           countdown store and before each of the three record accesses that
 *           follow it, and its countdown test reads the stored byte back rather
 *           than testing the decremented value. It takes no arguments, returns
 *           nothing and answers no value, and the 0x18-byte frame only keeps $ra
 *           at 0x10 across the calls. No other span of the extracted payload set
 *           is byte-identical to this 0xFC-byte body, so its statement order and
 *           operand forms were written from its own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalWorkSequenceAndAdvanceWorkByte2WhenRouteReachedElseStepRouteOnCountdownExpiryPlcharPlp56700_801CEB10(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    switch (D_1F800044[8]) {
      case 3:
        func_8014D8D4(0x40);
        break;
      case 5:
        func_8014D8D4(0x41);
        break;
    }
    D_1F800044[2] = D_1F800044[2] + 1;
  } else {
    D_1F800044[9] = D_1F800044[9] - 1;
    if (D_1F800044[9] == 0) {
      D_1F800044[9] = 2;
      D_1F800044[8] = D_1F800044[8] + D_1F800044[0xB];
      D_1F800044[8] = D_1F800044[8] & 7;
      func_8014D8D4(D_1F800044[8]);
    }
  }
}
