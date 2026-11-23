#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materializes the
 * cell at both access groups (lui $v1,0x1F80 / lw $v1,0x44($v1)) because the
 * byte stores through the loaded pointers may alias the cell, so each access
 * stays on the named cell rather than on a cached pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, and the name is its own
 * D_<address> form, so the binding needs no map row. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared battle helper of the main executable, outside this overlay's image.
 * The call site passes no argument register and never reads $v0 afterwards, so
 * the original constrains neither the argument widths nor the return type; the
 * three exact lifts of this body in bmagic/magic059, bmagic/magic126 and
 * bmagic/magic145 declare the same no-argument, no-return prototype. */
void func_801E5988(void);

/* @source 0x801EF340
 * @behavior Step handler of the shared overlay handler run at 0x801EF89C. The
 *           word at 0x801EF8AC, entry 4 of that code-pointer run and entry 2 of
 *           the dispatch base at 0x801EF8A4, holds this address, and no jal in
 *           this payload reaches it, so the table is its only incoming path: it
 *           runs with work byte 1 equal to 4 through the dispatcher at
 *           0x801EF0DC and with work byte 2 equal to 2 through the dispatcher at
 *           0x801EF120. The body reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once ahead of frame creation, because the increment
 *           needs no stack, adds one to that object's unsigned byte 0x9 at byte
 *           width (lbu / addiu $v0,$v0,1 / sb), re-reads the cell and subtracts
 *           one from unsigned byte 0xA the same way, then tests the stored value
 *           truncated to eight bits (andi $v0,$v0,0xFF / bnez), so the
 *           truncated register - not a reload - supplies the compared value.
 *           Only when byte 0xA reaches zero does it read the shared battle work
 *           record pointer at 0x801EB4E0, decrement that record's byte 0xB by
 *           one and invoke the shared helper at 0x801E5988, the store filling
 *           the call delay slot. Takes no arguments, returns nothing, reads or
 *           writes no other state, and the 0x18-byte frame only keeps $ra at
 *           0x10 across the call. This 120-byte span is byte-identical to
 *           0x801EF0A0 of bmagic/magic145/03, 0x801EFCE4 of bmagic/magic126/03
 *           and 0x801EF5C0 of bmagic/magic059/03, proven with one payload
 *           comparison, and those lifts carry this body's role and name; the
 *           twin at 0x801EF1D0 of bmagic/magic040/03 owns no source. Statement
 *           order, reload points and declaration style also follow this
 *           target's exact siblings decrementWorkByte9ThenDecrementGlobalWork
 *           ByteBAndAdvanceWorkByte2WhenZeroBmagicMagic00903_801EF814 and
 *           workAdvanceAt16Bmagic.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementCountdownGlobal(
    void) {
  g_battle_work[0x9] += 1;
  if (--g_battle_work[0xA] == 0) {
    D_801EB4E0[0xB] -= 1;
    func_801E5988();
  }
}
