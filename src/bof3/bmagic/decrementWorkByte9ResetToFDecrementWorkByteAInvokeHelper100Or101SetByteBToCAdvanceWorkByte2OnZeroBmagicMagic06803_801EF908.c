#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name the exact bmagic lifts of this family use for the
 * same main-executable cell. The original re-materialises the cell with its own
 * lui / lw pair before every access, because each read follows a store or a call
 * that may alias the cell, so the body keeps the cell at each source site
 * instead of caching it in a local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Main-executable helper at 0x8015D71C, invoked with the one immediate selector
 * 0x100 or 0x101 after the work object's byte 0xA counts down. Outside this
 * payload, so it keeps a raw origin-tagged prototype and no map row, and its
 * role stays address-anchored. */
extern void func_8015D71C(u16 arg0); /* @source 0x8015D71C @kind function */

/* @source 0x801EF908
 * @behavior Step handler published by the fourth word of this overlay's handler
 *           run at 0x801EFCC8: that word is entry 3 of the run the dispatcher at
 *           0x801EF73C indexes with the work record's byte 1, and the same word
 *           is entry 2 of the run the dispatcher at 0x801EF780 indexes with the
 *           record's byte 2. No jal word in this payload targets the address.
 *           Decrements the work object's unsigned byte 0x9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the stored value at
 *           byte width (andi 0xFF / bnez, the constant 0xF of the taken path
 *           filling the branch delay slot), so the truncation supplies the
 *           compared value and the byte is not re-read. Only when that countdown
 *           reaches zero it re-reads the cell and stores 0xF back into byte 0x9,
 *           then re-reads the cell and decrements byte 0xA the same byte-width
 *           way. That second countdown picks the helper selector: while it stays
 *           nonzero the handler calls 0x8015D71C with 0x100 and decrements work
 *           byte 2, and when it also reaches zero it calls 0x8015D71C with 0x101,
 *           stores the literal 0xC into work byte 0xB, and increments work byte
 *           2. The byte-2 store sits once after both arms, reached by the
 *           0x100 arm's j, and each arm re-reads the cell after its call or
 *           store because those accesses may alias it. Frame 0x18 keeps only
 *           $ra; takes no arguments, returns nothing and touches no other state.
 *           Field roles stay offset-anchored: bytes 9 and 0xA count down, 0xB
 *           holds the literal stored here, byte 2 is the selector the 0x801EF780
 *           dispatcher reads, and the helper's role is unestablished. No
 *           byte-identical copy of this 200-byte span exists in any payload under
 *           out/binaries: an exact scan and a scan masking every j/jal word both
 *           found the span only in this target's own binary, and a scan comparing
 *           every word's opcode and register fields found no shape variant of it
 *           either, so no sibling seeds these statements.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ResetToFDecrementWorkByteAInvokeHelper100Or101SetByteBToCAdvanceWorkByte2OnZeroBmagicMagic06803_801EF908(void) {
    if (--g_battle_work[9] == 0) {
        g_battle_work[9] = 0xF;
        if (--g_battle_work[0xA] != 0) {
            func_8015D71C(0x100);
            g_battle_work[2]--;
        } else {
            func_8015D71C(0x101);
            g_battle_work[0xB] = 0xC;
            g_battle_work[2]++;
        }
    }
}
