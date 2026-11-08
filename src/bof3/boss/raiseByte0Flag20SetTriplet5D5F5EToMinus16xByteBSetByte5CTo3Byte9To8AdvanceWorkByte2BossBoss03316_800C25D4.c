#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here and every access reaches it through this cell. Declared as a
 * named cell so each load folds to lui plus an offset load; the original re-reads
 * the cell before every store group because a store through the loaded pointer
 * may alias the cell, so every group names its own local. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Readiness poll helper of the main executable. The battle and bmagic lifts
 * declare the same prototype and discard or test its returned byte. */
extern u8 func_8014DAEC(void);

/* @source 0x800C25D4
 * @behavior Work-step handler at index 7 of this overlay's code-pointer run
 *           based at 0x800C27B4, which is the base word the step dispatcher
 *           func_800C1E70 (that run's word 0) loads and indexes with work byte 2
 *           scaled by four; run word 7 at 0x800C27D0 holds this address, and the
 *           run's row D_800C2744 ends with it. It loads the scratch work object
 *           published at the pointer cell 0x1F800044 and raises bit 0x20 of that
 *           object's byte 0, sets object byte 0x5C to 3, then writes the same
 *           value -(object byte 0xB << 4) to object bytes 0x5D, 0x5F and 0x5E in
 *           that store order, re-reading both the cell and byte 0xB for each of
 *           the three. It then sets object byte 0x9 to 8 and advances object
 *           byte 2 by one, and that store fills the delay slot of its single
 *           call, the readiness helper func_8014DAEC, whose returned byte this
 *           step discards. The next step of the same run, 0x800C26A4, repeats
 *           the 0x5D/0x5F/0x5E writes and decrements byte 0x9, so 8 is that
 *           countdown's start. Takes no arguments and returns nothing; reads or
 *           writes no other state. The roles of bytes 0x5C, 0x5D, 0x5E, 0x5F,
 *           0x9 and 0xB are not established, so the name describes the stores.
 *           No jal in this payload names 0x800C25D4; its only in-image
 *           reference is the run word at 0x800C27D0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseByte0Flag20SetTriplet5D5F5EToMinus16xByteBSetByte5CTo3Byte9To8AdvanceWorkByte2BossBoss03316_800C25D4(
    void) {
  u8 *flag;
  u8 *stage;
  u8 *x;
  u8 *z;
  u8 *y;
  u8 *timer;
  u8 *step;

  flag = D_1F800044;
  flag[0x0] |= 0x20;
  stage = D_1F800044;
  stage[0x5C] = 3;
  x = D_1F800044;
  x[0x5D] = -(x[0xB] << 4);
  z = D_1F800044;
  z[0x5F] = -(z[0xB] << 4);
  y = D_1F800044;
  y[0x5E] = -(y[0xB] << 4);
  timer = D_1F800044;
  timer[0x9] = 8;
  step = D_1F800044;
  step[2]++;
  func_8014DAEC();
}
