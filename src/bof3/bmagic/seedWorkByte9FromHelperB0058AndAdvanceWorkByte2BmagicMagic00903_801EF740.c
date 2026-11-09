#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044. The original re-materialises the
 * cell with its own lui/lw pair for both access groups (lui $v1,0x1F80 /
 * lw $v1,0x44($v1), load-delay nop) because the byte store through the loaded
 * pointer may republish the cell, so each access stays on the named cell rather
 * than on a cached pointer local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Out-of-image battle helper at 0x800B0058, which returns the battle
 * participant lookup byte for the current participant index. The overlay only
 * calls it, and the target's generated undefined-funcs map already spells the
 * address with this raw name, so this prototype carries the source tag alone
 * and needs no map row. */
extern u8 func_800B0058(void); /* @source 0x800B0058 */

/* @source 0x801EF740
 * @behavior Handler entry 0 of the nine-word pointer run at 0x801EEC04 whose
 *   tail 0x801EEC18..0x801EEC27 is the four-word table that 0x801EF6B0 copies
 *   into its frame and dispatches on scratchpad work byte 2, so this span runs
 *   for work byte 2 equal to 0. It calls the out-of-image helper at 0x800B0058
 *   with no arguments (jal, empty delay slot) and stores the returned byte at
 *   byte width into the work record's countdown byte at offset 0x9
 *   (sb $v0,0x9($v1)), then re-reads the scratchpad pointer cell at 0x1F800044
 *   with a second lui/lw pair, reads that record's work byte at offset 0x2,
 *   increments it (lbu / addiu $v0,$v0,1 / sb) and so advances the state that
 *   the table indexes. The second cell read is a reload rather than a live
 *   pointer because the store through the loaded pointer may republish the
 *   cell. The 0x18-byte frame only keeps $ra at 0x10 across the call, and the
 *   returned-byte store is scheduled into the frame restore. The span takes no
 *   arguments, returns nothing, reads or writes no other state, and its 0x48
 *   bytes end in jr $ra with the stack restore in the delay slot. No jal in
 *   this payload reaches it; the pointer word at 0x801EEC18 holds the address,
 *   which is the dispatch-table shape rather than a missing caller. The body is
 *   the same one recorded exact at 0x801F17BC and 0x801F133C in the magic008
 *   archive of this family: those spans differ from this one in exactly one
 *   word, the jal immediate, which names 0x800AFF98 there and 0x800B0058 here,
 *   so the shape is shared while this function keeps its own callee.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperB0058AndAdvanceWorkByte2BmagicMagic00903_801EF740(
    void) {
  u8 value;

  value = func_800B0058();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
