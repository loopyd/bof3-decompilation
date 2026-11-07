#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared overlay state byte. This step clears it when the record's five-tick
 * band counter passes its last band, and other overlay steps raise it.
 * @source 0x8014933B @kind data
 */
extern u8 D_8014933B;

/* Block drawer at 0x801F4E70, still unlifted assembly inside this target, so
 * only its prototype is declared. It draws the four-way block set that the
 * ramp and step handlers of this overlay select with a variant index. */
void func_801F4E70(u32 variant);

/* Drawing pass at 0x801F49A0, still unlifted assembly inside this target; the
 * sibling step handlers run it with pass 0 and pass 1 before the closing
 * drawer func_801F482C. */
void func_801F49A0(u32 pass);

/* @source 0x801F4030
 * @behavior Overlay step handler, entry 6 of the work-byte-2 handler table at
 *           0x801F5954. It reads the work record published at the scratchpad
 *           pointer cell 0x1F800044 and treats that record's byte 0x09 as a
 *           five-tick band counter: while the byte's remainder modulo five is
 *           zero it hands the block drawer func_801F4E70 the band index
 *           (byte 0x09 / 5) + 4. It then re-reads the cell, advances byte 0x09
 *           by one and runs the drawing pass func_801F49A0(1). A further cell
 *           read tests the advanced byte's band again: once it has reached
 *           band 3 (byte 0x09 = 0x0F through 0x13) the handler clears the
 *           shared overlay state byte at 0x8014933B, restarts byte 0x09 at 1
 *           and, through one more cell read, increments the record's work byte
 *           0x02 so the next frame dispatches entry 7 of the same table. Takes
 *           no arguments, returns nothing and uses a 0x18-byte frame that keeps
 *           the return address at 0x14 and, at 0x10, the register that holds the
 *           divisor constant across the drawing-pass call. The roles of the two
 *           callees beyond their drawing effects stay described by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkBandOnFifthTickAndAdvanceWorkByte2At15World02Area10313_801F4030(
    void) {
  u8* work;
  u8* tickCursor;
  u8* bandCursor;
  u8* stepCursor;
  u32 ticks;
  u32 bandTicks;

  work = D_1F800044;
  ticks = work[9];
  if ((u8)(ticks % 5) == 0) {
    func_801F4E70((u8)(ticks / 5) + 4);
  }
  tickCursor = D_1F800044;
  tickCursor[9]++;
  func_801F49A0(1);
  bandCursor = D_1F800044;
  bandTicks = bandCursor[9];
  if ((u8)(bandTicks / 5) == 3) {
    D_8014933B = 0;
    bandCursor[9] = 1;
    stepCursor = D_1F800044;
    stepCursor[2]++;
  }
}
