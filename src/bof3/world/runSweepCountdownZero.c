#include "bof3/bof3.h"

s32 rampIntensityIntegrateRender(
    void);

/* @source 0x801F2CD8
 * @behavior Entry 2 of this overlay nine-word state-handler run based at
 *           0x801F36CC, which
 *           callStateTable36World03 selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments; entry 1 at 0x801F2C80 arms that object byte at +0x9 with
 *           0x3C, so this body is the 60-frame stage that follows. It calls this
 *           overlay per-frame record sweep at 0x801F33B0, which walks the
 *           0x28-byte-stride record table at 0x800E4800, advances the intensity
 *           ramp, position and angle of every active record, emits each one
 *           through 0x801F2FC4 and leaves 1 in its return register once at least
 *           one record was active; this body discards that result. It then loads
 *           the scratchpad pointer cell once, counts that object byte at +0x9
 *           down by one as an unsigned-byte wrap add, so a byte already at zero
 *           stores 0xFF, and only when the stored byte is zero does it re-read
 *           the cell and advance the work byte at +0x1 by one, handing the next
 *           frame to entry 3 at 0x801F2D3C, which arms +0x9 with 0x96. The cell
 *           is re-read because the byte store through the loaded pointer may
 *           alias it. Takes no arguments and returns nothing, and the 0x18-byte
 *           frame only keeps the return address at 0x10. Representation notes:
 *           the slot-offset local and the two re-evaluated PSX_REF accesses are
 *           the spelling of the exact neighbouring handlers at 0x801F2C80 and
 *           0x801F2D3C. Spelling `SPAD_PTR_SLOT(u8, 0x44)` at both accesses
 *           instead scored 21 of 25 instructions, because the shared constant
 *           cell address was then commoned into one register as
 *           `lui a0,0x1f80` plus `ori a0,a0,0x44` before the branch, where the
 *           bytes at 0x801F2CE8 and 0x801F2D10 each rebuild `lui v1,0x1f80` and
 *           load the cell at 0x44; the local-offset form keeps both accesses
 *           folded and matched first time.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSweepCountdownZero(
    void) {
  s32 slot_offset;

  rampIntensityIntegrateRender();
  slot_offset = 0x44u;
  if (--PSX_REF(u8*, SPAD_BASE + slot_offset)[9] == 0) {
    PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
  }
}
