#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * row g_battle_work. A byte store through the loaded pointer may alias the
 * cell, so the compiler re-materialises the address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) once per statement that touches the work object, which is where
 * the original puts its five load pairs. The sibling handlers of this target
 * read the same cell through the same declaration. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. The sibling bodies
 * decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrementGlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic14403_801EF738
 * and incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic14403_801F00F0
 * read and test the same record byte 0xB through this pointer. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF6A4
 * @behavior Per-frame handler of the scratchpad work object, named only by the
 *           in-image handler-pointer run based at 0x801F04D0: the word at
 *           0x801F04F4 is the single word in this payload that holds this
 *           address, and no jal targets it, so the dispatcher that reads the
 *           run sits elsewhere and the table row proves the entry boundary and
 *           the no-argument ABI. It first reads the work-object pointer cell at
 *           0x1F800044 and increments work-object byte 9 modulo 256 (lbu /
 *           addiu 1 / sb). It re-materialises that cell with its own lui / lw
 *           pair, because the byte store through the loaded pointer may alias
 *           the cell, and increments work-object byte 0xA modulo 256 the same
 *           way. It re-reads the cell a third time and compares the reloaded
 *           byte 9 against 0x19, which the original materialises as addiu
 *           $v0,$zero,0x19, so the test is on the stored byte, not on the
 *           incremented register. Only on equality does it read the shared
 *           battle work record pointer at 0x801EB4E0, set bit 0x80 of that
 *           record's byte 0xB (lbu / ori 0x80 / sb, a read-modify-write of the
 *           stored byte), then re-read the work-object cell once more and
 *           increment work-object byte 2 modulo 256, advancing to the next
 *           byte-2 handler step. Leaf with no frame and no saved registers: it
 *           takes no arguments, returns nothing through the jr $ra at
 *           0x801EF730, and reads or writes no other state. The roles of work
 *           bytes 9, 0xA and 2 and of record byte 0xB are not established, so
 *           they stay offset-anchored.
 *
 *           The 0x94-byte span was byte-identical to func_801EF990 of
 *           emi/bmagic/magic092/03 (payload offset 0xD90): a payload cmp of the
 *           two spans reported zero differing bytes over 148 bytes against
 *           every extracted payload under out/binaries, and the only address
 *           constants either body carries are the shared scratchpad cell and
 *           the main-RAM record pointer, so there was no relocation to
 *           re-derive. That twin was still on its asm row when this span was
 *           lifted and so supplied no source to clone; the statement order,
 *           reload points, operand forms and offset spelling written here are
 *           this span's own bytes. The literals 0x19, 0x80 and the touched
 *           offsets are read from this payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9AndWorkByteAThenSetGlobalWorkByteBFlag80AndAdvanceWorkByte2At19BmagicMagic14403_801EF6A4(void) {
  g_battle_work[0x09]++;
  g_battle_work[0x0A]++;

  if (g_battle_work[0x09] == 0x19) {
    D_801EB4E0[0x0B] |= 0x80;
    g_battle_work[0x02]++;
  }
}
