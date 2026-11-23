#include "bof3/bof3.h"

s32 rampIntensityIntegrateRender(
    void);

/* Resident helper of the shared main executable that this overlay only calls:
 * it clears the work-area bytes at +0x00 through +0x04 of the object published
 * at the scratchpad pointer cell 0x1F800044. Declared here the way this
 * overlay's exact siblings declare the resident helpers they call. */
void func_80196070(void);

/* @source 0x801F2F90
 * @behavior Entry 8, the ninth and last word of this overlay's state-handler
 *           run based at 0x801F36CC, which
 *           callStateTable36World03 selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments. It runs this overlay's per-frame record sweep at
 *           0x801F33B0, which leaves 1 in its return register once at least one
 *           record of the 0x800E4800 table was active and 0 when none was, and
 *           only when the low byte of that result is zero does it call the
 *           resident main-executable helper at 0x80196070, which clears the
 *           work-area bytes at +0x00 through +0x04. The zero test is caller
 *           side: the callee returns a plain word, so the mask belongs to this
 *           body. This body is therefore the run's reset stage: while records
 *           stay active it only renders them through the sweep, and once the
 *           sweep reports none left it clears the work area instead. It reads
 *           and writes no state of its own, the 0x18-byte frame only keeps the
 *           return address at 0x10 across the call, and it makes no second
 *           call on the active path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateFlagsNoActive(void) {
  if ((rampIntensityIntegrateRender() &
       0xFF) == 0) {
    func_80196070();
  }
}
