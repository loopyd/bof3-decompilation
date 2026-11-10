#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0868
 * @behavior Decrements the scratchpad work object's unsigned byte 9 and,
 * when the stored byte becomes zero, reloads the work pointer and increments
 * byte 2. The pointer cell is at 0x1F800044. The byte-width decrement wraps,
 * and the store may alias the pointer cell, so the second access reloads it.
 * Takes no arguments and returns nothing. A payload table word at 0x801F0BD8
 * names this entry. A payload cmp proved all 76 bytes equal to the body at
 * emi/bmagic/magic110/03@0x801EF4DC; this seed keeps that body's expression
 * order and this target's existing g_battle_work binding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndAdvanceWorkByte2OnZeroBmagicMagic11303_801F0868(void) {
  if (--g_battle_work[9] == 0) {
    g_battle_work[2]++;
  }
}
