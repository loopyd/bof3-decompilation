#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. This
 * target's generated undefined_syms_auto.txt already binds the address, so no
 * target-map row or WEAK_SYMBOL_AT binding is needed; the byte view matches the
 * other bmagic lifts that gate on this record. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0E50
 * @behavior One handler step reached as entry 2 of the four-word handler run
 * at 0x801F1394 that the dispatcher at 0x801F0D64 selects with the scratchpad
 * work object's byte 0x2. Reads the shared battle work record pointer at
 * 0x801EB4E0 once and, only while that record's byte at offset 0x2 equals 0xA,
 * reads the scratchpad work-object pointer cell at 0x1F800044 and increments
 * the work object's byte at offset 0x2 with a byte-width add (lbu, addiu
 * $v0,$v0,1, sb through the same base). The compare constant is materialized
 * in $v0 after the record-byte load, so that load delay stays a nop, and the
 * cell is only touched on the taken path. Leaf with no frame; takes no
 * arguments, returns nothing and reads or writes no other state. Statement
 * order, operand forms and declaration style follow the exact sibling
 * incrementWorkByte2WhenGlobalWorkByteBIs84BmagicMagic11303_801EF8A4
 * (emi/bmagic/magic113/03@0x801EF8A4), whose 0x40 bytes are this function's
 * with guard offset 0x2 and compare constant 0xA in place of 0xB and 0x84.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByte2Is10BmagicMagic06403_801F0E50(void) {
  if (D_801EB4E0[0x2] == 0xA) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
