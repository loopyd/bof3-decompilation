#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by the battle and bmagic overlays
 * and bound at 0x1F800044 by this target's map. Kept as the named cell symbol
 * the exact siblings on this target use: the original re-materialises the base
 * for every access group (lui $v1,0x1F80 / lw $v1,0x44($v1) before the
 * increment, then the same pair into $v0 before each arm statement), because
 * the byte store through the loaded pointer may alias the cell. The plain cell
 * is kept rather than the volatile one several siblings use: every reload here
 * is already separated from the previous access by a store through the loaded
 * pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEE84
 * @behavior Per-frame state handler of the work object published at the
 *           scratchpad pointer cell 0x1F800044. Increments the object's
 *           unsigned byte at offset 0xA at byte width (lbu / addiu 1 / sb) and
 *           tests the truncated value it just stored (andi 0xFF of the stored
 *           register, no second load of the byte) against the bound 0x21
 *           unsigned (sltiu $v0,$v0,0x21 / bnez to the epilogue), so the arm
 *           runs only when byte 0xA has reached 0x21. In that arm it re-reads
 *           the pointer cell, increments the object's byte at offset 0x1 (lbu /
 *           addiu 1 / sb) - that byte is the state index the overlay's step
 *           dispatchers index into their handler runs, so this handler advances
 *           the state once byte 0xA passes 0x20 - then re-reads the cell a third
 *           time and clears the object's byte at offset 0x9 (sb $zero), the
 *           countdown byte the neighbouring handlers decrement. Leaf with no
 *           frame and no other state touched: it takes no arguments, returns
 *           nothing, and keeps the original's empty delay slots after each of
 *           the three cell loads and on the branch. The only in-image reference
 *           to 0x801EEE84 is the word at payload offset 0x8, the second entry of
 *           this overlay's leading pointer run 0x801EEDA0..0x801EF0BC; no jal in
 *           the payload reaches it, which is the dispatch-table shape rather
 *           than a missing caller. Statement order, operand forms, the saved
 *           pointer local for the first block and the u8 local for the truncated
 *           stored value follow the exact sibling
 *           add4ToWorkByteAThenSetWorkByteBToOneAndAdvanceWorkByte2WhenAtLeast20BmagicMagic15103_801F0C50,
 *           whose first block stores a local byte and then bound-tests that
 *           stored register; the arm statements copy this target's exact sibling
 *           incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0
 *           and the byte-9 clear of
 *           clearWorkBytes9AThenAdvanceWorkByte2BmagicMagic05903_801EF4C4, with
 *           this span's own operator, offsets, bound and statement order. No
 *           byte-identical copy of this 0x60-byte span exists in any of the 398
 *           payloads under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementByteClear(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[0xA] + 1;
    work[0xA] = value;
    if (value >= 0x21) {
        g_battle_work[1]++;
        g_battle_work[9] = 0;
    }
}
