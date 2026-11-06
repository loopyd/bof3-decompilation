#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original re-materializes and reloads the cell for each access
 * (lui $v0,0x1F80 / lw $v0,0x44($v0) before the increment, then the same pair
 * into $v1 for the arm), so the cell is reached through this target-map symbol
 * rather than through a constant address, which the compiler would materialize
 * once and reuse. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF0E0
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. Increments the object's unsigned byte 0xA
 *           with a byte-width read-modify-write (lbu / addiu 1 / sb) and
 *           compares the stored value with 0x60 (andi 0xFF / addiu $v0,
 *           $zero,0x60 / bne), so the truncation supplies the compared value
 *           and the byte is not re-read. Only when it reaches 0x60 the handler
 *           re-reads the cell and increments the object's byte 1 by one at byte
 *           width; that byte is the state index the overlay's step dispatcher
 *           at 0x801EED7C reads (lbu 1, sll 2) to load a word of the handler
 *           run at 0x801EFC9C and call it, so this handler advances the state
 *           after 0x60 frames. The second read of the cell is required: the
 *           store through the loaded work pointer may alias the cell. Leaf with
 *           no frame; it takes no arguments (the dispatcher's jalr sets up none
 *           and the body reads no argument register), returns nothing and
 *           touches no other state. The only in-image reference to 0x801EF0E0
 *           is the word at payload offset 0x10AC, the fifth entry of this
 *           overlay's six-word handler run 0x801EFC9C..0x801EFCB0, so the entry
 *           is dispatched and has no jal caller. Statement order, operand forms
 *           and declaration style follow the exact sibling
 *           decrementWorkByteAAndAdvanceWorkByte1OnZeroBmagicMagic10103_801EEF20
 *           and setWorkByte9To8SetWorkByteATo4AndIncrementWorkByte1BmagicMagic05203_801EEC7C,
 *           whose bodies differ only in the operator, the two byte offsets and
 *           the compared constant (0 here, 0x60 at this handler); no
 *           byte-identical copy of this span exists in any other overlay payload
 *           of this repository's binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0(void)
{
  u8 countdown;

  countdown = (u8)(g_battle_work[0xA] + 1);
  g_battle_work[0xA] = countdown;
  if (countdown == 0x60) {
    g_battle_work[1]++;
  }
}
