#include "bof3/bof3.h"

/* @source 0x801F4D44
 * @behavior Overlay state-entry reset: clears the lead byte of each of the 0x10
 *           entries of the 0x1C-byte-stride work table at 0x800E4800 (the
 *           main-RAM work area this overlay shares with the world and scenario
 *           overlays), walking the table with an unsigned byte counter in a
 *           bottom-tested loop, then zeroes the two overlay latch bytes at
 *           0x801F6510 and 0x801F6514. Both latches are read-then-set-once
 *           flags in this same payload: func_801F4F60 sets 0x801F6510 to 1 when
 *           it finds it zero, and func_801F4FEC does the same for 0x801F6514,
 *           so clearing them here re-arms those one-shot actions. It takes no
 *           arguments, returns nothing and reads no state. Its only caller
 *           inside this payload is
 *           statePhaseFrameCounter
 *           at 0x801F3E64, which after the
 *           call bumps the scratch work record's byte at offset 0x01, stores
 *           0x40 to that record's halfword at offset 0x2E, clears its halfword
 *           at offset 0x30 and queues cue 0x200. The loop half is the same
 *           instruction sequence as the already-exact
 *           stateClearEntryTable lift in this
 *           target (same base, walk and return), differing only in its two
 *           immediates (stride 0x1C versus 0x38, count 0x10 versus 0x18) and in
 *           the two latch stores that function does not carry, so it supplied
 *           the statement order and operand forms while the offset literals and
 *           the tail stores came from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateOneShotLatches(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x1Cu;
    i += 1u;
  } while (i < 0x10u);

  PSX_REF(u8, 0x801F6510u) = 0u;
  PSX_REF(u8, 0x801F6514u) = 0u;
}
