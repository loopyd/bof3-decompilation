#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Block drawer at 0x801F4E70, still unlifted assembly inside this target, so
 * only its prototype is declared. It draws the four-way block set that the
 * ramp and step handlers of this overlay select with a variant index. */
void func_801F4E70(u32 variant);

/* Drawing pass at 0x801F49A0, still unlifted assembly inside this target; the
 * sibling step handlers run it with pass 1 after advancing the band counter. */
void func_801F49A0(u32 pass);

/* Shared work-area reset helper of the main executable at 0x80196070 (the
 * overlay header include/bof3/world/area02713_internal.h records it as the
 * reset its work-trail stepper calls), which clears the scratchpad work-area
 * header this sequence restarts through. */
extern void func_80196070(void);

/* @source 0x801F42E0
 * @behavior Overlay step handler, the word at 0x801F5978 of the handler
 *           pointer run based at 0x801F5948 that the work-byte-1 dispatcher
 *           at 0x801F36D4 indexes. It reads the work record published at the
 *           scratchpad pointer cell 0x1F800044 and treats that record's byte
 *           0x09 as a five-tick band counter. Once the counter's band index
 *           (byte 0x09 divided by five, truncated to a byte) reaches 4, the
 *           handler hands the sequence to the shared work-area reset helper
 *           func_80196070 and advances nothing. Otherwise, on every tick whose
 *           byte 0x09 is a multiple of five, it hands the block drawer
 *           func_801F4E70 the descending band variant 7 - band index; it then
 *           re-reads the cell, advances byte 0x09 by one and runs the drawing
 *           pass func_801F49A0(1). Takes no arguments, returns nothing and uses
 *           a 0x18-byte frame that holds only the return address at 0x10. The
 *           roles of the two in-target callees beyond their drawing effects and
 *           of the handler pointer run stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkBandWithDescendingVariantOnFifthTickThenResetWorkAreaOnLastBandWorld02Area10313_801F42E0(
    void) {
  u8* work;
  u8* tickCursor;
  u32 ticks;
  u8 band;

  work = D_1F800044;
  ticks = work[9];
  band = (u8)(ticks / 5);
  if (band == 4) {
    func_80196070();
    return;
  }
  if ((u8)(ticks % 5) == 0) {
    func_801F4E70(7 - band);
  }
  tickCursor = D_1F800044;
  tickCursor[9]++;
  func_801F49A0(1);
}
