#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044. This target's map binds the
 * address as g_battle_work, so the cell keeps that name instead of a
 * fixed-address constant. The original re-materialises the cell for the
 * byte-1 access after the two calls, because a callee may repoint it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle target byte in main RAM, outside this overlay's image. The
 * original reads it as an unsigned byte and passes it zero-extended, and this
 * target's generated undefined_syms_auto.txt already binds the address, so the
 * name needs no map row. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared main-executable helper that raises battler status flags: its exact
 * callers pass this target byte with one flag word and it ORs that word into
 * the status of the battlers the byte selects. */
extern void func_800A9610(u8 arg0, s32 arg1); /* @source 0x800A9610 @kind function */

/* Shared main-executable helper of unresolved role. Its selector is the only
 * argument, and 0x101 is the value this span passes; the exact bmagic lift
 * subtractWorkWord20From14ThenSeedSixtyFourRecordsAndAdvanceWorkByte2
 * BmagicMagic12503_801EF960 calls it with the same selector, while the caller
 * of another overlay passes 0x100, so the role stays address-anchored. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C @kind function */

/* @source 0x801EEF54
 * @behavior Counts down work byte 0x9 through the scratchpad work-object
 *           pointer cell at 0x1F800044 with a byte-width read-modify-write
 *           (lbu, addiu -1, sb) and tests the truncated stored value (andi
 *           0xFF, bnez with an empty delay slot). Only when that countdown
 *           reaches zero it invokes the main-executable helper at 0x8015D71C
 *           with selector 0x101, then passes the shared battle target byte at
 *           0x80146384 (unsigned load, zero-extended) and the flag word 0x10
 *           to func_800A9610, which raises that flag on the battlers the byte
 *           selects, and finally reloads the work pointer cell and increments
 *           work byte 0x1 by one at byte width. Takes no arguments, returns
 *           nothing, and the 0x18-byte frame only keeps $ra at 0x10 across the
 *           two calls. The leading 0x1F800044 load is hoisted above the frame
 *           setup, which is the order the same-target sibling func_801EEFC4
 *           shows for the same cell. The span is entry 2 of the four-word
 *           pointer run at 0x801EEC04 (0x801EED28, 0x801EEE50, 0x801EEF54,
 *           0x801EEFC4), so the word at payload offset 0xC is its only
 *           reference and no jal in this payload reaches it: an overlay
 *           dispatch entry, not a called function, and the pointer word
 *           carries no prototype. The statement order, operand forms and
 *           declaration style follow the exact bmagic lift
 *           decrementWorkByte9RaiseBattleTargetFlag10AndAdvanceWorkByte1OnZero
 *           BmagicMagic13003_801EEE0C, whose 104-byte body is byte-identical
 *           to this span once the 0x101 call and its delay slot are removed
 *           (one cmp, and the only differing word is the shorter span's bnez
 *           displacement, which moves by exactly those 8 bytes); the extra
 *           call, its position ahead of the flag call and the selector 0x101
 *           come from this span's own bytes. The roles of work bytes 0x9 and
 *           0x1 and of selector 0x101 are not established, so they stay
 *           offset-anchored. A masked payload scan (call and branch immediates
 *           zeroed, little-endian) found no byte-identical twin: this span
 *           carries the selector 0x101 in the first call delay slot, which the
 *           family members that reach the same helper pass as 0x100 or 0x102.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9InvokeHelper101AndRaiseBattleTargetFlag10ThenAdvanceWorkByte1BmagicMagic21903_801EEF54(void) {
    if (--g_battle_work[9] == 0) {
        func_8015D71C(0x101);
        func_800A9610(D_80146384, 0x10);
        g_battle_work[1]++;
    }
}
