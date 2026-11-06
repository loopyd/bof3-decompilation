#include "bof3/bof3.h"

/* The overlay's work record is published through the scratchpad pointer cell at
 * 0x1F800044. It is bound as a named target-local symbol rather than through
 * SPAD_PTR_SLOT because this function re-reads the cell in four separate
 * groups: the constant-address macro let GCC compute the whole address once
 * (`lui a0,0x1f80` / `ori a0,a0,0x44`) and then reach every group through
 * `lw v1,0(a0)`, which measured a size-exact partial whose first difference was
 * the missing leading `lui v0,0x1f80`. The symbol makes the assembler
 * re-materialise `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` for each
 * load, which is what the original bytes show. The cell is a plain pointer: the
 * stores through the loaded record already separate the reads, so no volatility
 * is needed. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* Shared overlay helpers this step calls after its own update. 0x801F3BA0
 * builds the full-screen 0x140-by-0xF0 semi-transparent tile whose r0/g0/b0
 * bytes it reads from the work record's 0x5D/0x5E/0x5F triplet, and 0x801F3C84
 * walks the entity records and submits each through 0x8014D290. Both are still
 * unlifted assembly inside this target, so only their prototypes are declared. */
void func_801F3BA0(void);
void func_801F3C84(void);

/* @source 0x801F3A6C
 * @behavior Overlay flash-fade step of the work record published at the
 *           scratchpad pointer cell 0x1F800044. It first decrements the
 *           record's frame countdown byte +0x09 with a byte-width
 *           read-modify-write (`lbu`/`addiu -1`/`sb`). It then re-reads the
 *           cell and, while the countdown stays non-zero, subtracts 6 from the
 *           record's tint triplet at +0x5D, +0x5E and +0x5F (`lbu`/`addiu -6`/
 *           `sb` each), re-reading the cell before the +0x5E and +0x5F channels
 *           so those two channel groups each keep their own pointer load; once
 *           the countdown reads zero it instead stores 6 into the record's state
 *           byte +0x01 (`sb` of the constant the `beqz` delay slot loads) and
 *           skips the fade. Every path then calls 0x801F3BA0 and 0x801F3C84.
 *           The countdown is the 16 frames the sibling 0x801F3A14 seeds when it
 *           selects state byte 5 (it writes 0x10 into +0x09) and the triplet is
 *           the 0x60/0x60/0x60 white the sibling 0x801F384C seeds, so 16 steps
 *           of -6 fade it out; the state byte 6 stored here hands the next frame
 *           to the 0x801F3B18 step. The fade arm is written first because the
 *           original `beqz` jumps past it to the state store, so the state store
 *           is the `else`. The countdown group has its own cursor local: one
 *           shared pointer measured a size-exact partial that held both groups
 *           in $v1, while the original uses $v0 for the countdown group and $v1
 *           for the rest. Takes no arguments, returns nothing and keeps only the
 *           0x18-byte frame holding $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fadeWorkTintTripletBy6AndAdvanceWorkByte1To6WhenCountdownExpiresWorld02Area10814_801F3A6C(void) {
  u8* countdown;
  u8* work;

  countdown = workRecord44World02Area10814;
  countdown[9]--;
  work = workRecord44World02Area10814;
  if (work[9] != 0) {
    work[0x5D] -= 6;
    work = workRecord44World02Area10814;
    work[0x5E] -= 6;
    work = workRecord44World02Area10814;
    work[0x5F] -= 6;
  } else {
    work[1] = 6;
  }
  func_801F3BA0();
  func_801F3C84();
}
