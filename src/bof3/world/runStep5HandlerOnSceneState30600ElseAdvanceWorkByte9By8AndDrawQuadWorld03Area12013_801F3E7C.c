#include "bof3/bof3.h"

/* Step 5 of this same work-byte-2 handler run, at 0x801F3DF0, which its own
 * claimed source defines; only its prototype is declared here. It advances work
 * record byte 9, draws the quad from a ramp folded out of that byte and, once
 * the byte has reached 0x10, clears it and requests work-byte-2 handler 2.
 *
 * This body takes no arguments, so the original leaves the entry argument
 * registers untouched and the prototype stays void. */
void advanceWorkByte9DrawQuadWithCounterRampThenReturnToHandler2OnExpiryWorld03Area12013_801F3DF0(
    void);

/* Quad drawer of this target at 0x801F3F5C, which its own claimed source
 * defines; only its prototype is declared here. Its first argument biases the
 * pass rows, its second spreads the quad corners over y and then x, and its
 * fourth pulls the two leading corners down. */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
    u32 arg0, s32 arg1, u32 arg2, s32 arg3);

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load for every access and reloads it after
 * each store through the loaded record, so the source keeps one pointer local
 * per access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x801F3E7C
 * @behavior Step 6 of the work-byte-2 handler run this overlay dispatches
 *           through the table at 0x801F43E8, whose seventh pointer word names
 *           this body. It follows the work record published through the
 *           scratchpad pointer cell 0x1F800044 to the record pointer held at
 *           work offset 0x0C, masks the 32-bit word that pointer addresses to
 *           0x00FFFF00 and compares the masked word against the scene state
 *           0x30600. When the masked word reads exactly that value it clears
 *           work record byte 9, writes 5 into work record byte 0x02, the record
 *           byte the work-byte-2 dispatcher table indexes, and then runs step
 *           5's own body at 0x801F3DF0 in the same frame, so the ramp handler
 *           restarts from a cleared counter.
 *           Otherwise it advances work record byte 9 by 8, reads that byte back
 *           through a fresh load of the cell and folds two magnitudes from it.
 *           The first is 0x40 minus the byte taken as a magnitude and then
 *           raised by 0x40 again, and it fills the drawer's first argument. The
 *           second is 0x20 minus the halved byte taken as a magnitude and then
 *           raised by 0x20, and it fills the drawer's spread argument. Once the
 *           byte read back has reached 0x80 the handler also clears work byte 9
 *           and requests work-byte-2 handler 2 through the record byte at
 *           offset 0x02, which sends the run to 0x801F3B84. The drawer at
 *           0x801F3F5C is then called with (first magnitude, second magnitude,
 *           1, 0). Takes no arguments, returns nothing, and its 0x18-byte frame
 *           holds only the saved return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runStep5HandlerOnSceneState30600ElseAdvanceWorkByte9By8AndDrawQuadWorld03Area12013_801F3E7C(
    void) {
  u8* work;
  u8* readWork;
  u8* resetWork;
  u8 step;
  s32 wide;
  s32 spread;

  work = D_1F800044;
  if ((*(u32*)*(u32*)(work + 0xC) & 0xFFFF00) == 0x30600) {
    work[9] = 0;
    resetWork = D_1F800044;
    resetWork[2] = 5;
    advanceWorkByte9DrawQuadWithCounterRampThenReturnToHandler2OnExpiryWorld03Area12013_801F3DF0();
  } else {
    work[9] += 8;
    readWork = D_1F800044;
    step = readWork[9];
    wide = __builtin_abs(0x40 - step) + 0x40;
    spread = __builtin_abs(0x20 - (step >> 1)) + 0x20;
    if (step >= 0x80) {
      readWork[9] = 0;
      resetWork = D_1F800044;
      resetWork[2] = 2;
    }
    drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithArgSpreadAndByteARowWorld03Area12013_801F3F5C(
        wide, spread, 1, 0);
  }
}
