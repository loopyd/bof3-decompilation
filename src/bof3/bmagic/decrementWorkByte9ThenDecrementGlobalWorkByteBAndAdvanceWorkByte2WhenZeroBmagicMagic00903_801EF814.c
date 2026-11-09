#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044. The original
 * re-materializes the cell at both access groups (lui $v1,0x1F80 /
 * lw $v1,0x44($v1)) because the byte stores through the loaded pointers may
 * alias the cell, so each access stays on the named cell rather than on a
 * cached pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's
 * image. This target's generated undefined-syms map already binds the address
 * because the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF814
 * @behavior Reads the scratchpad work-object pointer cell at 0x1F800044
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), load-delay nop) and
 *           decrements the work object's unsigned countdown byte at offset
 *           0x9 with a byte-width read-modify-write (lbu $v0,0x9($v1) /
 *           addiu $v0,$v0,-1 / sb $v0,0x9($v1)), testing the stored value
 *           against zero (andi $v0,$v0,0xFF / bnez, empty delay slot). Only
 *           when that countdown reaches zero it loads the shared battle work
 *           record pointer at 0x801EB4E0 (lui $v1,%hi / lw $v1,%lo, load-delay
 *           nop) and decrements that record's byte at offset 0xB by one at
 *           byte width (lbu / addiu $v0,$v0,-1 / sb), then re-reads the
 *           scratchpad cell (a second lui/lw pair, same aliasing reason) and
 *           increments the work object's work byte at offset 0x2 at byte width
 *           (lbu / addiu $v0,$v0,1 / sb), advancing the dispatcher on work
 *           byte 2. Takes no arguments, returns nothing, keeps no frame and
 *           touches no other state; the 0x68-byte range is a leaf ending in a
 *           plain jr $ra with an empty delay slot. The word at 0x801EEC20, the
 *           eighth entry of the overlay's leading nine-word pointer run
 *           0x801EEC04..0x801EEC27, holds this address; no jal in this payload
 *           reaches it, which is the dispatch-table shape rather than a missing
 *           caller. The body follows this target's exact sibling
 *           decrementWorkByte9ThenIncrementWorkByte1WhenZeroBmagicMagic00903_801EF01C
 *           for the byte-9 countdown prelude and the re-read/advance tail, and
 *           the exact sibling decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4BmagicMagic09203_801F0420
 *           for the global-byte decrement in the zero arm, which here does not
 *           clear the work bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2WhenZeroBmagicMagic00903_801EF814(
    void) {
  if (--g_battle_work[9] == 0) {
    D_801EB4E0[0xB]--;
    g_battle_work[2]++;
  }
}
