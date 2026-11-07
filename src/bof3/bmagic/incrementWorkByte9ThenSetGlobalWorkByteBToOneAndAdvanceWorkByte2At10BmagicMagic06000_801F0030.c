#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by the battle and bmagic
 * overlays and bound at 0x1F800044 by this target's map. The symbolic address
 * folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two
 * separate reads each re-materialise the base. Kept as the named cell the exact
 * siblings on this target use; no call separates the second read from the store
 * that precedes it, so the plain cell already supplies the reload. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer outside this overlay's image. Splat already binds
 * the address through this target's generated undefined_syms_auto.txt, and the
 * byte written here is byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0030
 * @behavior Advances the scratchpad work object's byte at offset 0x9 by one
 *           modulo 256 through one load of the pointer cell at 0x1F800044
 *           (lbu / addiu 1 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x10. Only when that byte equals 0x10 does it read the
 *           shared battle work pointer at 0x801EB4E0, store 1 into that
 *           record's byte at offset 0xB - that constant is materialised in the
 *           delay slot of the equality test - re-read the pointer cell at
 *           0x1F800044 and increment the work object's byte at offset 0x2
 *           (lbu / addiu 1 / sb), advancing the overlay work byte 2 handler
 *           sequence. Leaf with no frame; takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           Its code pointer is entry 4 of the eleven-entry handler run at
 *           0x801F0BB0, which the work-byte-1 dispatcher of this overlay calls
 *           with no arguments, so it is reached through that table rather than
 *           by a call. The mirror sibling
 *           decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2
 *           WhenZeroBmagicMagic15103_801EFD24 (emi/bmagic/magic151/03
 *           @0x801EFD24) has the same statement order, pointer local and shared
 *           battle work store; this function's own operator and immediate
 *           (add 1, test 0x10) follow the sibling
 *           add2ToWorkByte9AndAdvanceWorkByte2At10BmagicMagic13103_801F01A8,
 *           whose first nine instructions match this function's but for the
 *           addend.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2At10BmagicMagic06000_801F0030(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[9] + 1;
    work[9] = value;
    if (value == 0x10) {
        D_801EB4E0[0xB] = 1;
        g_battle_work[2]++;
    }
}
