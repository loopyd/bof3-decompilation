#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound by this target's own map row
 * g_battle_work = 0x1F800044 and spelled that way by this target's other exact
 * lifts. This body reads the cell once and reuses the pointer for the word and
 * the byte access, so the single access stays one plain pointer load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8C0
 * @behavior Entry 9 of the fourteen-word work-byte-1 handler table based at
 *           0x801F046C and entry 1 of the six-word work-byte-2 run at 0x801F048C,
 *           which the payload selects with the scratchpad work object's byte 1
 *           and byte 2 and calls indirectly, so no jal names this span and its
 *           parameter roles stay structural. Reads the scratchpad work pointer
 *           cell at 0x1F800044 once (lui/lw), adds 0x40 to work word 0x14 and
 *           stores the sum back at word width, the store filling the branch delay
 *           slot, and only when that sum equals 0xC00 increments work byte 2
 *           modulo 256 with lbu/addiu/sb. The other exact lifts of this target
 *           read work word 0x14 as the scratchpad radius, which is the role the
 *           name states. Leaf with no frame: takes no arguments, returns nothing
 *           and touches no other state. No payload binary holds a byte-identical
 *           copy of the 0x38-byte span, so the statement order and operand forms
 *           come from these original words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRadiusItReaches(void) {
  u8 *work = g_battle_work;

  if ((*(u32 *)(work + 0x14) += 0x40) == 0xC00) {
    work[2]++;
  }
}
