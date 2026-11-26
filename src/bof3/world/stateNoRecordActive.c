#include "bof3/bof3.h"

s32 rampIntensityIntegrateRender(
    void);

/* @source 0x801F2E10
 * @behavior Entry 5 of this overlay's nine-word state-handler run based at
 *           0x801F36CC, which
 *           dispatchByte1Table36cc selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments. It runs this overlay's per-frame record sweep at
 *           0x801F33B0, which leaves 1 in its return register once at least one
 *           record of the 0x800E4800 table was active and 0 when none was, and
 *           only when the low byte of that result is zero does it advance the
 *           same work byte at +0x1, handing the next frame's dispatch to entry
 *           6 at 0x801F2E58. The zero test is caller side: the callee returns a
 *           plain word, so the mask belongs to this body. It reads and writes
 *           nothing beyond that one byte; the 0x18-byte frame only keeps the
 *           return address at 0x10 across the call, and the pointer cell is
 *           loaded once on the zero path alone. Representation notes: the cell
 *           is spelled with the SPAD_PTR_SLOT(u8, 0x44) macro, the form this
 *           overlay's exact single-access handlers use, which issues the
 *           original five-word increment sequence (lui 0x1F80, lw 0x44,
 *           lbu 0x1, addiu 0x1, sb 0x1) seen at 0x801F2E2C..0x801F2E44.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateNoRecordActive(void) {
  if ((rampIntensityIntegrateRender() &
       0xFF) == 0) {
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
  }
}
