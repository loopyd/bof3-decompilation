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

/* @source 0x801CE514
 * @behavior Work byte 2 == 1 step of this overlay handler run: the word at
 *           0x801D0790 is entry 1 of the twenty-four-word run based at
 *           0x801D078C, which the byte-2 dispatcher at 0x801CE404 indexes by
 *           work byte 0x02, and entry 0 of that run at 0x801CE448 stores the
 *           chosen candidate route index in byte 0x03 together with the step
 *           sign in byte 0x0B, rearms the countdown byte 0x09 to 2 and
 *           increments byte 0x02, which is how this body is reached. It takes
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and compares the current route index byte 0x08 with that
 *           stored route byte 0x03. When the two agree the route has been
 *           reached, so it selects the shared work sequence through
 *           func_8014D8D4 with 0x40 for route 3 and 0x41 for route 5, calls the
 *           selector on neither for any other route value, then increments the
 *           step counter byte 0x02, which advances the dispatch to entry 2 of
 *           the same run, the word 0x801B2D54 at 0x801D0794 that lies outside
 *           this payload. Otherwise it decrements the countdown byte 0x09 and
 *           returns without advancing; only when that reloaded byte reaches
 *           zero does it store 2 back into the countdown, add the step sign
 *           byte 0x0B into the route index byte 0x08, mask the route index to
 *           the low three bits of the eight-entry direction ring and select the
 *           shared work sequence named by that new route index. The original
 *           re-reads the pointer cell after the countdown store and before each
 *           of the three record accesses that follow it, and its countdown test
 *           reads the stored byte back rather than testing the decremented
 *           value. It takes no arguments, returns nothing and answers no value,
 *           and the 0x18-byte frame only keeps $ra at 0x10 across the calls.
 *
 * Shape note: the 0xFC bytes of this span were compared against the exact lift
 * of the same payload at 0x801CEB10 and differ in three words only - the route
 * test at span offset 0x2C, which is a branch-not-equal here and a
 * branch-equal there, and the two intra-body jumps at offsets 0x34 and 0x60,
 * whose absolute targets carry the 0x5FC span delta, the first one further
 * displaced by one instruction because the measured sibling branches to the
 * shared call on the last matched route and this body falls through to it.
 * Statement order, operand forms and declaration style were taken from that
 * measured exact lift, and the measured outcome of the two-route selection then
 * fixed its one remaining difference: written with case 5 before case 3, the
 * switch compares the sorted route values 3 and then 5 while it lays the case-5
 * body out as the fallthrough, which is the branch pair this span shows,
 * whereas case 3 first measured 61 of 63 instructions with its first difference
 * on that same test, and an if/else-if chain measured 55 of 63 in 0xF8 bytes
 * with its first difference on the same word. Its literals, offsets, record
 * access points and jump targets are its own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalReachedExpiry(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    switch (D_1F800044[8]) {
      case 5:
        func_8014D8D4(0x41);
        break;
      case 3:
        func_8014D8D4(0x40);
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
