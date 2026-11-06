#include "bof3/bof3.h"

void func_801F55F0(u8 arg0);

/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;
/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F4F0C
 * @behavior Overlay gate: when the shared mode byte at 0x80143BB0 equals 5
 *           (`lbu`/`li 5`/`bne`) the handler at 0x801F55F0 runs with 0
 *           (`addu a0,zero,zero` in the `jal` delay slot) and jumps to the
 *           epilogue. Every other value loads the record published through the
 *           global pointer cell 0x80148210 (`lw v1,%lo(D_80148210)`) and
 *           subtracts 2 from its +0x0A halfword (`lhu`/`addiu -2`/`sh`), with
 *           the record address rematerialised once and the halfword read and
 *           written back through that same register. Takes no arguments and
 *           returns nothing; the roles of the record and of the callee stay
 *           anchored to their addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementRecordHalfword0aUnlessMode5World02Area10814_801F4F0C(void) {
  if (D_80143BB0 == 5) {
    func_801F55F0(0);
  } else {
    D_80148210[5] -= 2;
  }
}
