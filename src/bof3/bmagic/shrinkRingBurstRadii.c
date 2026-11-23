#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, shared by all overlays and
 * already bound as g_battle_work by this target's map row. The original
 * re-materializes the cell base for each access (lui $v1,0x1F80 / lw $v1,68($v1)
 * before the countdown read and again before the byte-1 update), so the cell
 * stays the named symbol rather than a pointer local that would fold the two
 * reads into one. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Ring radius of the effect transform at 0x801EFA9C, the halfword at +0x10.
 * The overlay init 0x801EECA4 zeroes it, the handler 0x801EEE24 ramps it up by
 * 0x40 per frame, the handler 0x801EEEC4 republishes it as 0x100 or 0x110, and
 * this span ramps it down by 0x40. The sixteen-vertex ring draw 0x801EF040
 * loads it signed and scales the rotator answers with it, but this span loads
 * it unsigned (lhu) and stores the decremented halfword back, so it is declared
 * unsigned here. */
extern u16 D_801EFAAC; /* @source 0x801EFAAC @kind data */

/* Burst radius of the same transform, the next halfword at +0x12: the burst
 * particle draw 0x801EF4E0 loads it signed and multiplies the rotator answers
 * for both axes by it, offsetting the origin words 0x801EFAA0 and 0x801EFAA4. The init zeroes it, the handler 0x801EEE24 adds 0x60 per frame,
 * and this span subtracts 0x60. Like the ring radius it is loaded unsigned
 * (lhu) here and stored back as a halfword. */
extern u16 D_801EFAAE; /* @source 0x801EFAAE @kind data */

/* Draw setup of this overlay at 0x801EF254, still an asm row of this target's
 * Splat listing and therefore already defined by the link. It binds the tpage,
 * installs the draw mode on the primitive cursor and runs the overlay's draw
 * passes against the state above, so this call is the frame's whole draw and it
 * takes no arguments and returns nothing. */
void func_801EF254(void); /* @source 0x801EF254 */

/* @source 0x801EEF80
 * @behavior Magic-150 effect handler, the entry at 0x801EEC18 of the overlay
 *           handler table 0x801EEC04 and index 5 of its seven words, which the
 *           dispatcher 0x801EEC20 indexes by unsigned work byte 1, so it runs
 *           once per frame while the dispatcher sits on this step. It shrinks
 *           the two radii of the effect transform in fixed steps, subtracting
 *           0x40 from the ring radius at 0x801EFAAC and 0x60 from the burst
 *           radius at 0x801EFAAE with an unsigned halfword load, one addiu and a
 *           halfword store each, so the ring and the burst particles contract
 *           together. It then calls the overlay draw setup 0x801EF254 with no
 *           arguments, so the frame is drawn once from the state it just wrote.
 *           After the draw it decrements work byte 9 with the stored byte masked
 *           to 0xFF for the test and returns untouched while that countdown is
 *           nonzero; when it reaches zero it re-reads the work pointer and
 *           increments work byte 1, which moves the dispatcher on to the next
 *           handler entry, and it neither re-arms the countdown nor submits a
 *           cue. Those are its only two stores, it reads no cell other than the
 *           work pointer and the two radii, it has no loop, and its 0x18-byte
 *           frame keeps $ra at 0x10 across the single call. It takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void shrinkRingBurstRadii(
    void) {
  /* The original keeps the ring radius address in a register across its load
   * and its store (lui $a0,0x801F / addiu $a0,$a0,%lo(D_801EFAAC) before the
   * frame's $ra store, then lhu 0($a0) and sh 0($a0)), so this access goes
   * through a local pointer; the direct symbol form folds to a lui plus a
   * %lo-displacement pair per access and loses that base. The burst radius is
   * a direct symbol access in the original and stays one here. */
  u16 *ring_radius = &D_801EFAAC;

  *ring_radius -= 0x40;
  D_801EFAAE -= 0x60;
  func_801EF254();
  if (--g_battle_work[9] == 0) {
    g_battle_work[1]++;
  }
}
