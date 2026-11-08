#include "bof3/bof3.h"

/* Front-end record pointer cell of the shared main executable. The body clears
 * this record's state byte 0x12B when the countdown expires; no other byte of
 * that record is read here, and the record's role is established elsewhere in
 * this overlay.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* World-flag halfword of the shared main executable. The body clears bit 0x1000
 * of it. No lift of this executable names that flag yet, so the halfword and
 * the bit stay address-anchored.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address with `lui` plus an offset load before it creates the frame and keeps
 * the loaded pointer in a saved register across both calls, so the source keeps
 * the named-symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Work advance step of the shared main executable, outside this payload: it
 * adds the work record's current delta into its coordinate and counter words
 * and then runs the shared update helper on its own condition. The original
 * sets nothing up in the argument registers and reads no result from $v0. */
void func_801B0DFC(void);

/* Shared no-argument update helper of the main executable, outside this
 * payload. The original sets nothing up in the argument registers and reads no
 * result from $v0. */
void func_8014D978(void);

/* @source 0x801CF564
 * @behavior Overlay step reached from outside this payload. It reads the
 *           unsigned countdown byte 0x09 of the active work object through the
 *           scratchpad pointer cell 0x1F800044. While that byte is nonzero it
 *           stores the value one lower and then runs the work advance step
 *           func_801B0DFC, which moves the record by its current delta; when
 *           the byte reads zero it clears the state byte 0x12B of the front-end
 *           record published at 0x80146250 and clears bit 0x1000 of the
 *           world-flag halfword 0x80146258, leaving that byte and that flag
 *           address-anchored. Both paths then run the shared no-argument update
 *           helper func_8014D978. The body takes no arguments, returns nothing
 *           and answers no value. Its first load precedes frame creation, the
 *           0x18-byte frame only keeps $ra at 0x10 across the calls, the
 *           decrement is computed in the delay slot of the branch that skips
 *           the release path, and the slots after the countdown load and after
 *           the two word reads stay nops. The 0x70-byte body is word for word
 *           the lifted body at 0x801CE83C in the plp27a overlay, whose only
 *           differing word is the absolute jump immediate of the shared tail
 *           label, so this row is the same shape at this overlay's address and
 *           not a byte-identical clone. Neither this function's address nor any
 *           pointer to it appears in this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStateByte12BAndWorldFlagBit1000WhenWorkCountdownExpiresElseDecrementByte9AndAdvanceWorkPlcharPlp02500_801CF564(
    void) {
  u8* work;
  u8 remaining;

  work = D_1F800044;
  remaining = work[9];
  if (remaining == 0) {
    D_80146250[0x12B] = 0;
    D_80146258 &= 0xEFFF;
  } else {
    work[9] = remaining - 1;
    func_801B0DFC();
  }
  func_8014D978();
}
