#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by the battle and bmagic
 * overlays and bound at 0x1F800044 by this target's map. The symbolic address
 * folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two
 * separate reads each re-materialise the base. Kept as the named cell the exact
 * siblings on this target use; the store into the battle work record at
 * 0x801EB4E0 separates the second read from the first, so the plain cell
 * already supplies the reload. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer outside this overlay's image. Splat already binds
 * the address through this target's generated undefined_syms_auto.txt, and the
 * byte written here is byte-addressed, so the local declaration keeps the byte
 * view only. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F031C
 * @behavior Advances the scratchpad work object's byte at offset 0x9 by one
 *           modulo 256 through one load of the pointer cell at 0x1F800044
 *           (lbu / addiu 1 / sb), then tests the truncated byte it just stored
 *           (andi 0xFF of the stored register, no second load of the cell)
 *           against 0x1E. Only when that byte equals 0x1E does it read the
 *           shared battle work pointer at 0x801EB4E0, store 2 into that
 *           record's byte at offset 0xB - that constant is materialised in the
 *           delay slot of the equality test - re-read the pointer cell at
 *           0x1F800044 and increment the work object's byte at offset 0x2
 *           (lbu / addiu 1 / sb), advancing the overlay work byte 2 handler
 *           sequence. Leaf with no frame; takes no arguments, returns nothing
 *           and reads or writes no other state.
 *
 *           Its code pointer is entry 8 of the eleven-entry handler run at
 *           0x801F0BB0, which the work-byte-1 dispatcher of this overlay calls
 *           with no arguments, so it is reached through that table rather than
 *           by a call. The exact sibling
 *           incrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2At10BmagicMagic06000_801F0030
 *           has the same statement order, pointer local and shared battle work
 *           store; this function's own compared immediate (0x1E) and stored
 *           constant (2) follow the original bytes, and no other byte of the
 *           96-byte body differs from that sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenSetGlobalWorkByteBTo2AndAdvanceWorkByte2At1EBmagicMagic06000_801F031C(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[9] + 1;
    work[9] = value;
    if (value == 0x1E) {
        D_801EB4E0[0xB] = 2;
        g_battle_work[2]++;
    }
}
