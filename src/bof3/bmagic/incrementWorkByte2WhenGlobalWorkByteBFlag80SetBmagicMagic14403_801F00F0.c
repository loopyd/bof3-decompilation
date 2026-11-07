#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row or WEAK_SYMBOL_AT binding is needed. The sibling bmagic
 * lifts read the same record through this pointer
 * (clearWorkFlag40AndCopyRecordTripletClearByte9ThenAdvanceWorkByte2BmagicMagic14403_801F0034
 * copies its words 0x34/0x38/0x3C into the work object behind 0x1F800044). */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. It is a named cell
 * symbol, the form the exact siblings use: the symbolic address lets the
 * assembler fold it into the load (lui $v1,0x1F80 / lw $v1,68($v1)). This body
 * only touches the cell after the guard, so it needs no reload split. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F00F0
 * @behavior Overlay step handler, entry 13 of the work-byte-1 handler table
 *           based at 0x801F04D0. Reads the shared battle work record through
 *           the pointer at 0x801EB4E0 and, only when that record's byte at
 *           offset 0xB has bit 0x80 set (lbu / andi 0x80 / beqz, so the
 *           original tests the masked byte rather than a value equality), reads
 *           the scratchpad work-object pointer cell at 0x1F800044 once and
 *           increments its byte at offset 0x2 with a byte-width add
 *           (lbu / addiu 1 / sb). The guard runs before the scratchpad cell is
 *           touched, so 0x1F800044 is only dereferenced on the taken path.
 *           Leaf with no frame; nothing is returned and no other state is read
 *           or written. The 68-byte body is byte-identical to the exact lift
 *           incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic09203_801F03DC,
 *           which supplies this shape and name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic14403_801F00F0(void) {
  if (D_801EB4E0[0xB] & 0x80) {
    g_battle_work[2]++;
  }
}
