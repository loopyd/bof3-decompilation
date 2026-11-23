#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by the battle and bmagic overlays
 * and bound at 0x1F800044. Kept as the named cell symbol the exact siblings on
 * this target use: the original re-materialises the base for every access group
 * (lui $v1,0x1F80 / lw $v1,0x44($v1) before the byte-9 increment, then the same
 * pair into $v0 before each arm statement) because the byte store through the
 * loaded pointer may alias the cell. The plain cell is kept rather than the
 * volatile one several siblings use: every reload here is already separated from
 * the previous access by a store through the loaded pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEEE4
 * @behavior Per-frame state handler of the work object published at the
 *           scratchpad pointer cell 0x1F800044. Increments the object's
 *           unsigned byte at offset 0x9 at byte width (lbu / addiu 1 / sb) and
 *           tests the truncated value it just stored (andi 0xFF of the stored
 *           register, no second load of the byte) against the bound 0x21
 *           unsigned (sltiu $v0,$v0,0x21 / bnez to the epilogue), so the arm
 *           runs only once byte 0x9 has reached 0x21, the same 0x21 ceiling the
 *           work byte A of the sibling handler at 0x801EEE84 carries. In that
 *           arm it re-reads the pointer cell and increments the object's byte at
 *           offset 0x1 (lbu / addiu 1 / sb) - that byte is the state index the
 *           overlay's step dispatchers index into their handler runs, so this
 *           handler advances the state once byte 0x9 passes 0x20 - then
 *           re-reads the cell a third time and clears the object's byte at
 *           offset 0x9 (sb $zero), the same byte it just counted up. Leaf with
 *           no frame and no other state touched: it takes no arguments, returns
 *           nothing, and keeps the original's empty delay slots after each of
 *           the three cell loads and on the branch. The only in-image reference
 *           to 0x801EEEE4 is the word at payload offset 0xC (0x801EEC0C, the
 *           third entry of this overlay's leading pointer run
 *           0x801EEDA0..0x801EF0BC); no jal in the payload reaches it, which is
 *           the dispatch-table shape rather than a missing caller. Statement
 *           order, operand forms, the saved pointer local for the first block
 *           and the u8 local for the truncated stored value follow this target's
 *           exact sibling
 *           incrementByteClearBmagic
 *           (0x801EEE84), whose 0x60-byte span differs from this one in exactly
 *           two payload bytes - the byte-offset immediate of the lbu and of the
 *           matching sb in the first block (0xA there, 0x9 here) - so its body
 *           is transcribed with this span's own offset. That sibling measured
 *           exact on this target; no byte-identical copy of this 0x60-byte span
 *           exists in any payload under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementAdvanceClearBmagic(void) {
    u8 *work;
    u8 value;

    work = g_battle_work;
    value = work[9] + 1;
    work[9] = value;
    if (value >= 0x21) {
        g_battle_work[1]++;
        g_battle_work[9] = 0;
    }
}
