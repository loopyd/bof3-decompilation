#include "bof3/bof3.h"

/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F5608
 * @behavior Overlay step handler: four independent gates re-read the +0x79
 *           mode byte of the work record published through the global pointer
 *           cell 0x80146250 and advance the halfword at offset 0x0A of the
 *           record published through the global pointer cell 0x80148210 by the
 *           step that mode selects - 3 for mode 5, 6 for mode 4 and 9 for mode
 *           6. Mode 8 takes the step of zero and so rewrites the halfword with
 *           the value it already holds. Every gate that does not match leaves
 *           the halfword alone, each gate reloads the 0x80148210 record and
 *           re-reads the halfword, and the body keeps no stack frame. The only
 *           payload word equal to this address is an entry of the handler
 *           pointer run held in the overlay data blob, and no jal immediate in
 *           this payload names it, so an outside caller installs the body as a
 *           callback and its parameter roles stay structural. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateWorkByte79Step(void) {
  if (D_80146250[0x79] == 8) {
    D_80148210[5] = D_80148210[5];
  }
  if (D_80146250[0x79] == 5) {
    D_80148210[5] += 3;
  }
  if (D_80146250[0x79] == 4) {
    D_80148210[5] += 6;
  }
  if (D_80146250[0x79] == 6) {
    D_80148210[5] += 9;
  }
}
