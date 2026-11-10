#include "bof3/bof3.h"

/* Quad drawer of this target at 0x801F3F5C, lifted after this handler, so only
 * its prototype is declared here. Its head adds the third argument to the work
 * record byte at 0x0A and wraps it modulo 0x0C, and its body builds a textured
 * four-point polygon through SetPolyFT4, SetShadeTex and RotTransPers4. Its
 * fourth argument is subtracted from the horizontal and vertical extents it
 * derives from the work halfwords 0x36 and 0x3A. */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
    u32 arg0, u32 arg1, u32 arg2, u32 arg3);

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address with `lui` plus an offset load for every access, reloads it after
 * each store through the loaded pointer, and keeps the three loads in separate
 * registers, so the source keeps the named-symbol form with one pointer local
 * per access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F3DF0
 * @behavior Step 5 of the work-byte-2 handler run this overlay dispatches
 *           through the table at 0x801F43E8. It post-increments work record
 *           byte 9 and reads that byte back through a fresh load of the
 *           scratchpad pointer cell 0x1F800044, so the value it works from is
 *           the incremented one. From that byte it folds a ramp: the byte is
 *           subtracted from 8 and the difference is taken as a magnitude, so
 *           the ramp peaks where the byte passes 8, and that magnitude is
 *           subtracted from 8 again, negated and shifted left by four, which
 *           makes the fourth argument of the quad drawer at 0x801F3F5C run 0 at
 *           byte 0, -0x80 at byte 8 and back to 0 at byte 0x10. The call itself
 *           passes the fixed arguments (0x80, 0x40, 1) beside that ramp. Once
 *           the byte read back has reached 0x10 the handler also clears work
 *           byte 9 and requests work-byte-2 handler 2 through the record byte at
 *           offset 0x02, so the counter restarts and the run continues at
 *           0x801F3B84. Takes no arguments, returns nothing, and its 0x18-byte
 *           frame holds only the saved return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9DrawQuadWithCounterRampThenReturnToHandler2OnExpiryWorld03Area12013_801F3DF0(
    void) {
  u8* work;
  u8* readWork;
  u8* resetWork;
  s32 magnitude;
  s32 ramp;
  u8 step;

  work = D_1F800044;
  work[9]++;
  readWork = D_1F800044;
  step = readWork[9];
  magnitude = 8 - step;
  magnitude = __builtin_abs(magnitude);
  ramp = 8 - magnitude;
  if (step >= 0x10) {
    readWork[9] = 0;
    resetWork = D_1F800044;
    resetWork[2] = 2;
  }
  drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
      0x80, 0x40, 1, -ramp << 4);
}
