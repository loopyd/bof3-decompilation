#include "bof3/bof3.h"

void func_801F3268(s32 count);

s32 rampIntensityIntegrateRender(
    void);

/* @source 0x801F2D6C
 * @behavior Entry 4 of this overlay nine-word work-state handler run based at
 *           0x801F36CC, which
 *           callStateTable36World03 selects with
 *           the unsigned work byte at offset +0x1 of the work object published
 *           at the scratchpad pointer cell 0x1F800044 and calls with no
 *           arguments; entry 3 at 0x801F2D3C arms that object byte at +0x9 with
 *           0x96, so this body is the ten-step stage that follows. On the frame
 *           that enters it reads that countdown byte, divides it by 0x0F and
 *           keeps the remainder as an unsigned byte; when the remainder is zero
 *           it calls this overlay shuffled ring spawn at 0x801F3268 with the
 *           record count 2, which fills a stack list with the slot bytes 0 to 15,
 *           shuffles it with rand and claims that many ring records, so the
 *           armed 0x96 frames carry ten of those pairs, at the countdown values
 *           0x96, 0x87 and on down to 0x0F. It then runs this overlay per-frame
 *           record sweep at 0x801F33B0, which walks the 0x28-byte-stride record
 *           table at 0x800E4800, advances the intensity ramp, position and angle
 *           of every active record, emits each one through 0x801F2FC4 and
 *           leaves 1 in its return register once at least one record was active;
 *           this body discards that result. It then re-reads the cell, counts
 *           that byte at +0x9 down by one, and only when the stored byte is zero
 *           does it re-read the cell once more and advance the work byte at +0x1
 *           by one, handing the next frame to entry 5 at 0x801F2E10; the
 *           countdown therefore lasts 150 frames and the state advance happens
 *           on the frame whose decrement reaches zero, so 0x0F is the last
 *           spawn value and no wrap over 0xFF is taken. Each access re-reads the
 *           cell because the byte store through the loaded pointer may alias it.
 *           It takes no arguments, returns nothing, and its 0x18-byte frame only
 *           keeps the return address at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnEveryFifteenthSweep(
    void) {
  u32 slot_offset;
  u8 phase;

  slot_offset = 0x44u;
  phase = PSX_REF(u8*, SPAD_BASE + slot_offset)[9] % 0x0F;
  if (phase == 0) {
    func_801F3268(2);
  }
  rampIntensityIntegrateRender();
  if (--PSX_REF(u8*, SPAD_BASE + slot_offset)[9] == 0) {
    PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
  }
}
