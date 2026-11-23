#include "bof3/bof3.h"

void renderRadialBurstPoint(u8* point);

extern u8* volatile D_1F800044;

/* @source 0x801F30EC
 * @behavior State-6 step handler of this overlay work-byte-1 dispatcher (word 6
 *           of the handler table at 0x801F4624): it captures the scratchpad work
 *           record published at pointer cell 0x1F800044 and draws that record's
 *           point at +0xC once through
 *           renderRadialBurstPoint,
 *           then grows the point halfword at +0x6 by 2, lowers the point y
 *           halfword at +0x2 by the grown value and draws that same point a
 *           second time, so the two draws emit the ring at the old height and
 *           again at the reduced one. Finally it re-reads the pointer cell into
 *           a second local, decrements the work countdown byte at +9 and, once
 *           that byte reaches zero, advances the state byte at +1 through a
 *           third read of the cell. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderBurstGrowingLowering(
    void) {
  u8* work;
  u8* point;
  u8* count_work;

  work = D_1F800044;
  point = work + 0xCu;
  renderRadialBurstPoint(point);
  FIELD_REF(u16, work, 0x12u) += 2;
  FIELD_REF(u16, work, 0xEu) -= FIELD_REF(u16, work, 0x12u);
  renderRadialBurstPoint(point);
  count_work = D_1F800044;
  if (--count_work[9] == 0) {
    D_1F800044[1]++;
  }
}
