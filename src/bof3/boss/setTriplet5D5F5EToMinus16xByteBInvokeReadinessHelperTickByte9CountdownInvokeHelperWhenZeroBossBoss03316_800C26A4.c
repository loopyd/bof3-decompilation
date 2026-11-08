#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load; the original re-reads
 * the cell before every store group because a store through the loaded pointer
 * may alias the cell, so every group names its own local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype and discard or test its returned byte; this step
 * discards it. */
extern u8 func_8014DAEC(void);

/* Spent-countdown helper of the main executable, reached by every bmagic and
 * battle countdown step that fires when its timer reaches zero. Bound by the
 * generated undefined function row; takes no arguments and returns nothing. */
void func_801E5988(void);

/* @source 0x800C26A4
 * @behavior Work-step handler at index 8, the last entry, of this overlay's
 *           code-pointer run based at 0x800C27B4, which is the base word the
 *           step dispatcher func_800C1E70 loads and indexes with work byte 2
 *           scaled by four; run word 8 at 0x800C27D4 holds this address and the
 *           two-word tail run based at 0x800C27D0 ends with it. It loads the
 *           scratch work object published at the pointer cell 0x1F800044 and
 *           writes the same value -(object byte 0xB << 4) to object bytes 0x5D,
 *           0x5F and 0x5E in that store order, re-reading both the cell and byte
 *           0xB for each of the three. The last of those three stores fills the
 *           delay slot of its first call, the readiness helper func_8014DAEC,
 *           whose returned byte this step discards. It then re-reads the cell,
 *           counts object byte 0x9 down by one as an unsigned-byte wrap add, and
 *           only when that byte was already zero, so the countdown stores 0xFF,
 *           calls the spent-countdown helper func_801E5988. The countdown store
 *           happens on both paths, so it fills the spent-count branch delay
 *           slot. This step completes the 0x5D/0x5F/0x5E write group that step 7
 *           at 0x800C25D4 opens and spends the count of 8 that step seeds, and
 *           it advances no work byte. Takes no arguments and returns nothing;
 *           the 0x18-byte frame only keeps the return address at 0x10 across its
 *           two calls. The roles of bytes 0x9, 0xB, 0x5D, 0x5E and 0x5F are not
 *           established, so the name describes the stores and calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setTriplet5D5F5EToMinus16xByteBInvokeReadinessHelperTickByte9CountdownInvokeHelperWhenZeroBossBoss03316_800C26A4(
    void) {
  u8 *x;
  u8 *z;
  u8 *y;
  u8 *work;
  u8 value;

  x = D_1F800044;
  x[0x5D] = -(x[0xB] << 4);
  z = D_1F800044;
  z[0x5F] = -(z[0xB] << 4);
  y = D_1F800044;
  y[0x5E] = -(y[0xB] << 4);
  func_8014DAEC();
  work = D_1F800044;
  value = work[0x9];
  work[0x9] = value + 0xFF;
  if (value == 0) {
    func_801E5988();
  }
}
