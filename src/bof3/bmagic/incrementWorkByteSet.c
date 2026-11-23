#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * row g_battle_work. The exact lift
 * decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4BmagicMagic14403_801F0134
 * reads the same cell through the same declaration. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF11C
 * @behavior Per-frame handler of the scratchpad work object, named only by the
 *           in-image handler-pointer run based at 0x801F04D0: the word at
 *           0x801F04DC, entry 3 of the fifteen non-zero words of that run, is
 *           the single word in this payload that holds this address, and no jal
 *           in the payload targets it, so the dispatcher that reads the run
 *           sits outside this overlay's image. It reads the work-object pointer
 *           cell at 0x1F800044 once, before the guard, and keeps that pointer in
 *           $v1 across it. When bit 0x80 of the work object's byte at offset 0xB
 *           is set (lbu / andi 0x80 / beqz, so the original tests the masked
 *           byte rather than a value equality), it increments the byte at offset
 *           0x2 with a byte-width add (lbu / addiu 1 / sb) through that same
 *           pointer register. Leaf with no frame and no saved registers: takes no
 *           arguments and returns nothing through the jr $ra at 0x801EF14C, and
 *           reads or writes no other state. The roles of work bytes 0xB and 0x2
 *           are not established, so they stay offset-anchored. The 56-byte span
 *           is byte-identical to the reviewed exact lift
 *           incrementWorkByte2WhenWorkByteBFlag80SetBmagicMagic09203_801EF350
 *           (0x801EF350, target emi/bmagic/magic092/03, payload offset 0x750),
 *           which supplied this statement order and name; the sibling reaches
 *           the same cell through its own SPAD_PTR_SLOT spelling, and this
 *           target binds the cell by the map name instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByteSet(void) {
  u8 *work = g_battle_work;

  if (work[0xB] & 0x80) {
    work[2]++;
  }
}
