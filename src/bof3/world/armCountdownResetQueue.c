#include "bof3/bof3.h"

/* Shared sound-cue dispatcher of the main executable (lifted in
 * src/bof3/audio/dispatchSoundCue.c), cue id 0x201 of this overlay. */
extern void dispatchSoundCue(u32 cue_id); /* @source 0x8015DF18 @kind unknown */

/* Work record pointer cell of the scratchpad. */
extern u8* volatile D_1F800044;

/* @source 0x801F2E48
 * @behavior State-2 reset handler of this overlay work-byte-1 dispatcher (its
 *           pointer is word 2 of the handler table at 0x801F4624): it reads the
 *           scratchpad work record published at pointer cell 0x1F800044, arms
 *           that record's countdown byte at +9 with 0x5A, zeroes the halfwords
 *           at +0x10 and +0x12 - the step and growth fields the state-1 handler
 *           0x801F2DA0 steps by -2/+2 and the state-3 handler 0x801F2E98 climbs
 *           to 0x40 - and then advances the state byte at +1, which hands the
 *           record to the state-3 handler, and queues the front-end sound cue
 *           0x201. The record pointer is re-read from the cell between the byte
 *           store and the halfword stores, so the first pointer keeps the +0x10
 *           and +0x12 stores while the second serves the state increment,
 *           matching the two separate lui/lw pairs the original keeps. Takes no
 *           arguments and returns nothing; the 0x18-byte frame only keeps $ra
 *           at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armCountdownResetQueue(
    void) {
  u8* work;
  u8* state_work;

  work = D_1F800044;
  work[9] = 0x5A;
  state_work = D_1F800044;
  FIELD_REF(u16, work, 0x10u) = 0;
  FIELD_REF(u16, work, 0x12u) = 0;
  state_work[1]++;
  dispatchSoundCue(0x201);
}
