#include "bof3/bof3.h"

/* Slot update routine at 0x801EF8EC, still an asm row in this target's Splat
 * listing. Its own bytes return $a1, which is set to 1 only on a pass that
 * processed an active entry of the overlay table 0x801EFAB4 and stays 0 when
 * every entry was idle; this span tests that answer's low byte. */
extern u32 func_801EF8EC(void); /* @source 0x801EF8EC */

/* Scratchpad work-object pointer cell 0x1F800044, shared by all overlays and
 * bound as g_battle_work by this family's map row in the sibling target
 * emi/bmagic/magic151/03. The original re-materializes the cell base for each
 * access (lui $vN,0x1F80 / lw $vN,68($vN) before both the byte-9 store and the
 * byte-1 update), so the cell stays the named symbol rather than a pointer
 * local that would fold the second read into the first. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEDCC
 * @behavior Magic-150 effect handler, the entry at 0x801EEC0C of the overlay
 *           handler table that starts at 0x801EEC04, so index 2 of its seven
 *           words. The body creates a 0x18-byte frame with $ra at +0x10, calls
 *           the slot update at 0x801EF8EC with no arguments, truncates the
 *           answer to its low byte and returns untouched while that byte is
 *           nonzero. When the answer reads zero it re-reads the work-object
 *           pointer from scratchpad cell 0x1F800044, stores the constant 4 into
 *           work-object byte 9 through that pointer, re-reads the same cell and
 *           increments work-object byte 1, which moves the overlay handler step
 *           on to the next table entry. Those are the only two stores: no other
 *           cell is written or read, there is no loop, and the function returns
 *           nothing. The asm neighbour 0x801EED60 uses the same cell pair with
 *           the same frame and the same byte-1 advance, decrementing byte 9 per
 *           pass and advancing the step once that count reaches zero, which
 *           reads byte 9 as the overlay hold count and byte 1 as its step index;
 *           the dispatcher 0x801EEC20 confirms the step role by indexing the
 *           seven-word table at 0x801EEC04 with work-object byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepEffectSlotsThenSetWorkByte9To4AndAdvanceWorkByte1WhenNoneActiveBmagicMagic15003_801EEDCC(
    void) {
  if ((u8)func_801EF8EC() == 0) {
    g_battle_work[9] = 4;
    g_battle_work[1]++;
  }
}
