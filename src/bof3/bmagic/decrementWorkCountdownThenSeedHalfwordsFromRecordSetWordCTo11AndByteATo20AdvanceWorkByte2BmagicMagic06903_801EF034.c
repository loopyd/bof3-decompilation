#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The symbolic address folds into each load (lui $vN,0x1F80 /
 * lw $vN,68($vN)), so every separate read of the cell re-materialises the base
 * and keeps the original's four loads. The cell is deliberately not volatile
 * and no local is carried across a store group: a store through the loaded
 * pointer may alias the cell, which is what forces the reload the original
 * makes before each later group. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * other asm handlers of this target (0x801EF0C4, 0x801EF1D0, 0x801EF2AC and
 * the rest of the work-byte-1 table) load the same cell, so the target's
 * generated undefined_syms_auto.txt already binds the address and no map row
 * is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF034
 * @behavior Work-byte-1 state 3 handler of this overlay: entry 3 of the
 *           ten-entry handler table at 0x801F04F8, which the byte 1 dispatcher
 *           at 0x801EEE90 indexes with the scratchpad work object's state byte
 *           0x1. It reads the work-object pointer cell at 0x1F800044 once and
 *           decrements the object's countdown byte 0x9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb), then tests the truncated
 *           byte it just stored (andi 0xFF of the stored register, no second
 *           load). Only when that byte reached zero does the body run: it
 *           re-reads the pointer cell and, through that fresh pointer, stores
 *           0xB into work word 0xC, seeds work halfword 0x2E with halfword
 *           0x2E of the shared battle work record read through 0x801EB4E0 plus
 *           0x37, clears the countdown byte 0x9, and seeds work halfword 0x30
 *           with the record's halfword 0x30 minus 0x10. It then re-reads the
 *           cell to store 0x20 into work byte 0xA and re-reads it once more to
 *           advance work byte 0x2 with a byte-width read-modify-write
 *           (lbu/addiu/sb), which selects table entry 4 (0x801EF0C4). The cell
 *           is re-read before each later store group because the stores through
 *           the loaded pointer may alias it. The cell load that serves the
 *           whole first group is issued before the record load, so the record
 *           load's delay slot holds the word 0xC store, and the record's 0x30
 *           halfword is loaded above the byte 0x9 clear; naming that halfword
 *           in its own local reproduces both orders. Leaf with no frame; takes
 *           no arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownThenSeedHalfwordsFromRecordSetWordCTo11AndByteATo20AdvanceWorkByte2BmagicMagic06903_801EF034(void) {
    u8 *work;
    u8 *record;
    u8 *seeded;
    u16 *source;
    u16 *halves;
    u16 value;
    u8 countdown;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        seeded = g_battle_work;
        halves = (u16 *)seeded;
        record = D_801EB4E0;
        source = (u16 *)record;
        ((u32 *)seeded)[3] = 0xB;
        halves[0x17] = source[0x17] + 0x37;
        value = source[0x18];
        seeded[9] = 0;
        halves[0x18] = value - 0x10;
        g_battle_work[0xA] = 0x20;
        g_battle_work[2]++;
    }
}
