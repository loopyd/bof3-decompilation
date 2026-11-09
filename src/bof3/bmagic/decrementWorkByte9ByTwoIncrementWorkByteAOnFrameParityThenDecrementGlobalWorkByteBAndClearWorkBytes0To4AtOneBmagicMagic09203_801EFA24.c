#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's
 * map as g_battle_work. A byte store through the loaded pointer may alias the
 * cell itself, so the compiler re-materializes the address (lui $vN,0x1F80 /
 * lw $vN,68($vN)) once for the byte-9 countdown, once for the parity-guarded
 * byte-0xA increment, once for the byte-9 equality test and once for each of
 * the five trailing byte clears, which is where the original puts those nine
 * load pairs. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. The exact lift
 * decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4
 * BmagicMagic09203_801F0420 reads the same record through this pointer and
 * decrements the same byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared per-frame counter in main RAM, outside this overlay's image. The
 * shared map owns the address (config/targets/shared/symbols.txt), so this
 * target needs no row for it; its low bit is the per-frame phase that the
 * bmagic sibling lifts read, and it is relocated here as %hi/%lo. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* @source 0x801EFA24
 * @behavior Per-frame handler of the scratchpad work object reached only
 *           through the in-image handler-pointer run based at 0x801F07BC:
 *           dispatchStateByte1Table07bcBmagicMagic09203_801EF000 (0x801EF000)
 *           reads work byte 1, scales it by four and jalr's that entry, and the
 *           word at 0x801F07E4, entry 10 of the fifteen non-zero words in that
 *           run, holds this address and is the only word in the payload that
 *           does, so there is no jal caller. It first decrements the work
 *           object's byte 9 by two at byte width (lbu / addiu -2 / sb) through
 *           the single read of the pointer cell at 0x1F800044. It then tests
 *           bit 0 of the shared frame counter at 0x80143E6C (lui %hi / lw %lo /
 *           andi 0x1) and, when that bit is set, re-reads the pointer cell and
 *           increments byte 0xA modulo 256. Finally it re-reads the cell and
 *           the byte it stored earlier, not the stored register, because the
 *           intermediate byte-0xA store may replace the pointer itself, and
 *           compares that reloaded byte against 1, which the original
 *           materializes as addiu $v0,$zero,1. Only on equality does it read
 *           the shared battle work record pointer at 0x801EB4E0, decrement that
 *           record's byte 0xB by one at byte width, and clear work bytes 0x00
 *           through 0x04, each clear re-reading the pointer cell so the
 *           original shows five separate load pairs. Leaf with no frame and no
 *           saved registers: takes no arguments, returns nothing through the
 *           jr $ra at 0x801EFB00 and reads or writes no other state. The roles
 *           of work bytes 9, 0xA and 0x00..0x04 and of record byte 0xB are not
 *           established, so they stay offset-anchored. The 224-byte span is
 *           byte-identical to 0x801EF738 in emi/bmagic/magic144/03, proven with
 *           one payload comparison over every out/binaries payload; that row is
 *           still an unlifted raw row, so the twin is shape evidence rather
 *           than a clone source. The statement order and operand forms above
 *           the byte-0xA increment follow the reviewed exact lift
 *           incrementWorkByteAOnFrameParityThenAdd16ToWorkByte9AndAdvanceWork
 *           Byte2At80BmagicMagic00603_801F020C, and the five byte clears and
 *           the record decrement follow the reviewed exact lift
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4
 *           BmagicMagic09203_801F0420 of this same target.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrementGlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic09203_801EFA24(void) {
  g_battle_work[0x09] -= 2;

  if (D_80143E6C & 1u) {
    g_battle_work[0x0A]++;
  }

  if (g_battle_work[0x09] == 1) {
    D_801EB4E0[0x0B]--;
    g_battle_work[0x00] = 0;
    g_battle_work[0x01] = 0;
    g_battle_work[0x02] = 0;
    g_battle_work[0x03] = 0;
    g_battle_work[0x04] = 0;
  }
}
