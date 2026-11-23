#include "bof3/bof3.h"

/* Shared main-program helper of the boss overlays; the same address is the call
 * target of this payload's init body at 0x800C1848. Its role is not established
 * here, and the caller below masks the returned word down to one byte, so the
 * declared result is the wider word the original masks.
 */
s32 func_8014D978(void); /* @source 0x8014D978 */

/* @source 0x800C193C
 * @behavior Entry 0 of the byte-3 handler run this overlay bases at 0x800C1EBC,
 * the seven-word group inside its reviewed `textbin` row D_800C1E14:
 * callWorkByte3State scales the work record's
 * unsigned byte 3 by four and calls that entry with no arguments, and the
 * table's own void-returning type is what this body returns. The word 0x800C193C
 * at 0x800C1EBC is its only in-image reference and no jal in the payload reaches
 * it, so the run itself is its caller and the argument roles stay structural. It
 * calls the shared main-program helper func_8014D978 with no arguments, keeps the
 * low byte of the returned word, and while that byte is nonzero writes 1 into the
 * work record's byte at offset 3 - the state index the byte-3 dispatcher reads -
 * through the scratchpad pointer cell 0x1F800044; it writes nothing when the
 * masked byte is zero. The 0x18-byte frame only keeps $ra at 0x10 across the
 * call, and the cell is loaded once for the single store, whose load delay slot
 * stays the original nop. The stored 1 stays in $v0 because nothing is live in
 * that register past the handler return, so the original materializes it in the
 * taken-branch delay slot and reuses it for the store. The helper's role and the
 * meaning of its nonzero low byte are not established here, so the name describes
 * the recorded effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setByte3OnHelper(void) {
  if (func_8014D978() & 0xFF) {
    SPAD_PTR_SLOT(u8, 0x44u)[3] = 1;
  }
}
