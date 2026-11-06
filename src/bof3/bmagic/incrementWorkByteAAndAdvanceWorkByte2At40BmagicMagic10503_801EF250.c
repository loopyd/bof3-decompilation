#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original re-materializes and reloads the cell for each access
 * (lui $v0,0x1F80 / lw $v0,0x44($v0) before the increment, then the same pair
 * into $v1 for the arm), so the cell is reached through this target-map symbol
 * rather than through a constant address, which the compiler would materialize
 * once and reuse. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF250
 * @behavior Per-frame handler of the work object published at the scratchpad
 *           pointer cell 0x1F800044. Increments the object's unsigned byte 0xA
 *           with a byte-width read-modify-write (lbu / addiu 1 / sb) and
 *           compares the stored value with 0x40 (andi 0xFF / addiu $v0,
 *           $zero,0x40 / bne), so the truncation supplies the compared value
 *           and the byte is not re-read. Only when it reaches 0x40 does the
 *           handler read the cell again and increment the object's byte 2 by
 *           one at byte width; that second read is required because the store
 *           through the loaded work pointer may alias the cell. Leaf with no
 *           frame and no saved register; it reads no argument register,
 *           returns nothing and touches no other state. The only in-image
 *           reference to 0x801EF250 is the word at payload offset 0xC inside
 *           the raw entry's leading tabledata segment, so the payload holds no
 *           jal to this handler. Statement order, operand forms and
 *           declaration style follow the exact sibling
 *           incrementWorkByteAAndAdvanceWorkByte1At60BmagicMagic10803_801EF0E0,
 *           whose body differs only in the compared constant (0x60 there,
 *           0x40 here) and in the advanced object byte (1 there, 2 here); no
 *           byte-identical copy of this span exists in any overlay payload of
 *           this repository's binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteAAndAdvanceWorkByte2At40BmagicMagic10503_801EF250(void)
{
  u8 countdown;

  countdown = (u8)(g_battle_work[0xA] + 1);
  g_battle_work[0xA] = countdown;
  if (countdown == 0x40) {
    g_battle_work[2]++;
  }
}
