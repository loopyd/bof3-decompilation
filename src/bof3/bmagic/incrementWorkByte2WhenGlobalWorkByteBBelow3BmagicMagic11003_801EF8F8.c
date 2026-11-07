#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Both
 * this body and the rest of the target's remaining assembly (0x801EEC94,
 * 0x801EF6B4, 0x801EF740, 0x801EF93C and others) read the record through it,
 * so the address stays bound by the generated undefined-syms map of this
 * target and needs no target-map row. The byte-array type is the one the
 * byte-identical sibling incrementWorkByte2WhenGlobalWorkByteBBelow3
 * BmagicMagic22003_801EF888 declares, because this body reads one record byte. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's own map names it
 * g_battle_work, and its two existing exact lifts declare the same name, so
 * the guarded path dereferences it under that name: the original
 * re-materialises the base once (lui $v0,0x1F80 / lw $v0,68($v0)) after the
 * guard is taken. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8F8
 * @behavior Entry 2 of the six-word work-byte-2 handler run at 0x801F048C
 *           (0x801EF844, 0x801EF8C0, 0x801EF8F8, 0x801EF93C, 0x801F0040,
 *           0x801F00D8), which 0x801EF7B4 selects with the scratchpad work
 *           object byte 2 and calls indirectly; the same six words are the tail
 *           of the fourteen-word byte-1 table at 0x801F046C. It reads the
 *           shared battle work pointer at 0x801EB4E0 once and compares work
 *           record byte 0xB against three with an unsigned bound
 *           (lbu/sltiu), so the guarded path runs for every value below three.
 *           Only on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 once and increments the byte at offset 2 modulo 256 with
 *           lbu/addiu/sb. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state. Bytes
 *           0x801EF8F8..0x801EF93B are byte-identical to the exact siblings
 *           incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic06203_801EF5D8,
 *           incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic08203_801EF680
 *           and incrementWorkByte2WhenGlobalWorkByteBBelow3
 *           BmagicMagic22003_801EF888, so the statement order, operand forms and
 *           the guard-before-scratchpad load come from those bodies; the
 *           scratchpad cell keeps this target's own mapped name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic11003_801EF8F8(void) {
  if (D_801EB4E0[0xB] < 3u) {
    g_battle_work[2]++;
  }
}
