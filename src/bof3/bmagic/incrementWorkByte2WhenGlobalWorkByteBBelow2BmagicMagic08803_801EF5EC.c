#include "bof3/bof3.h"

/* Shared battle work pointer outside this overlay, already resolved by its
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF5EC
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares its byte at offset 0xB against two with an unsigned bound
 *           (lbu/sltiu), so the guard is taken for every value below two. Only
 *           on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 once and increments the byte at offset 2 of that work
 *           object modulo 256 with lbu/addiu/sb. Bytes
 *           0x801EF5EC..0x801EF62F are entry 3 of the five-entry pointer run at
 *           0x801F0028 (0x801EF524, 0x801EF900, 0x801EF59C, 0x801EF5EC,
 *           0x801EF630), and they are byte-identical to four unlifted spans:
 *           0x801EF148 of bmagic/magic223/03, 0x801EFBA8 of bmagic/magic004/03,
 *           0x801EF078 of bmagic/magic126/03 and 0x801EF410 of
 *           bmagic/magic167/03. They differ in one immediate from the exact
 *           sibling
 *           incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic06203_801EF5D8,
 *           which bounds the same record byte below three; statement order,
 *           operand forms and declaration style follow that sibling. Takes no
 *           arguments, returns nothing and keeps no frame, so the scratchpad
 *           cell is dereferenced only when the guard succeeds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic08803_801EF5EC(void) {
  if (D_801EB4E0[0xB] < 2u) {
    SPAD_PTR_SLOT(u8, 0x44u)[2]++;
  }
}
