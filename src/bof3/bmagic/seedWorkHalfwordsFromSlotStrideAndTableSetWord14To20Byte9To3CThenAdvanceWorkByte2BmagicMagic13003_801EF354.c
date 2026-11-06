#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. The symbolic address folds into each load (lui $vN,0x1F80 /
 * lw $vN,68($vN)), so every separate read of the cell re-materialises the base
 * and keeps the original's three loads. The cell is deliberately not volatile
 * and the first group's pointer is carried in one local: a store through the
 * loaded pointer may alias the cell, which is what forces the reload the
 * original makes before each of the two later store groups. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Six halfword slot offsets closed by the original's single index load: the
 * table sits at the end of the reviewed data row that starts at 0x801EFA44 and
 * its six entries are 0x64, 0x6C, 0x74, 0x74, 0x6C, 0x64. The step body reads
 * it with one halfword load at the work slot byte, so it stays address-anchored
 * until a consumer proves a narrower role than the slot index. */
extern u16 D_801EFA6C[]; /* @source 0x801EFA6C @kind data */

/* @source 0x801EF354
 * @behavior Work-byte-2 state 0 handler of this overlay: entry 0 of the
 *           six-entry handler table based at 0x801EFA5C, which the per-frame
 *           step dispatcher at 0x801EF284 indexes with the scratchpad work
 *           object's state byte 0x2. It reads the work-object pointer cell at
 *           0x1F800044 once and seeds the whole first store group through that
 *           pointer: work halfword 0x2E becomes the slot byte 0xB scaled by 26
 *           plus 0x98, work halfword 0x30 becomes the halfword table at
 *           0x801EFA6C indexed by the same slot byte 0xB plus 0x50, work word
 *           0x14 becomes 0x20 and work byte 0x9 becomes 0x3C. The slot byte is
 *           read twice and the build places the second read before the 0x2E
 *           store, as the original does. It then re-reads the pointer cell to
 *           clear work byte 0xA and re-reads it once more to advance work byte
 *           0x2 with a byte-width read-modify-write (lbu / addiu 1 / sb in the
 *           $ra delay slot), which hands the next frame to table entry 1
 *           (0x801EF3DC). Leaf with no frame; takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHalfwordsFromSlotStrideAndTableSetWord14To20Byte9To3CThenAdvanceWorkByte2BmagicMagic13003_801EF354(void) {
    u8 *work;
    u16 *halves;
    u32 *words;

    work = g_battle_work;
    halves = (u16 *)work;
    words = (u32 *)work;
    halves[0x17] = work[0xB] * 26 + 0x98;
    halves[0x18] = D_801EFA6C[work[0xB]] + 0x50;
    words[5] = 0x20;
    work[9] = 0x3C;
    g_battle_work[0xA] = 0;
    g_battle_work[2]++;
}
