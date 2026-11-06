#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * and by every exact sibling of this handler in the bmagic overlay family. It
 * stays a named cell rather than the constant-address memory accessor because
 * the original re-materialises the base before each access (lui $v1,0x1F80 /
 * lw $v1,68($v1)): the accessor's constant address is folded once by CSE into a
 * full-address register and reused, which loads the cell through that one base
 * instead of reloading it after the stores. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF44C
 * @behavior Per-frame step of the work object published at the scratchpad
 *           pointer cell 0x1F800044. It decrements the object's unsigned byte
 *           0xA with a byte-width read-modify-write (lbu / addiu -1 / sb) and
 *           compares the stored value with zero (andi 0xFF / bnez), so the
 *           truncation supplies the compared value and the byte is not
 *           re-read. Only when that byte reaches zero it re-reads the cell and
 *           stores the literal 0x3E back into byte 0xA, whose constant the
 *           original schedules into the bnez delay slot, then re-reads the cell
 *           once more and increments the object's byte 1 by one at byte width.
 *           Byte 1 is the state index the overlay entry at 0x801EEC18 reads
 *           (lbu 1, sll 2) to call a word of the five-entry code pointer run
 *           copied from 0x801EEC04, so this handler reloads the counter with 62
 *           frames and advances the state after every second of waiting. The
 *           two extra reads of the cell are required: the stores through the
 *           loaded work pointer may alias the cell. Leaf with no frame; it
 *           takes no arguments, returns nothing and touches no other state. The
 *           only in-image reference to 0x801EF44C is the word at payload offset
 *           0xC, entry 2 of this overlay's leading five-word handler run
 *           0x801EEC04 (0x801EEC88, 0x801EED50, 0x801EF44C, 0x801EF4A8,
 *           0x801EF4EC), so the entry is dispatched and has no jal caller.
 *           Statement order, operand forms and declaration style follow the
 *           exact sibling
 *           decrementWorkByteAAndAdvanceWorkByte1OnZeroBmagicMagic10103_801EEF20,
 *           whose body decrements the same byte 0xA and advances the same byte 1
 *           with the same statement order and differs only in the reset store
 *           this handler adds inside the arm before the increment; no
 *           byte-identical copy of this 92-byte span exists in any overlay
 *           payload under out/binaries/emi.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteAResetTo3EAndAdvanceWorkByte1OnZeroBmagicMagic06803_801EF44C(void) {
    if (--g_battle_work[0xA] == 0) {
        g_battle_work[0xA] = 0x3E;
        g_battle_work[1]++;
    }
}
