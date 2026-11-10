#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * row g_battle_work. A byte store through the loaded pointer may alias the
 * cell, so the compiler re-materializes the address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) once for the byte-9 subtraction, once for the parity-guarded
 * byte-0xA increment, once for the byte-9 equality test and once for each of
 * the five trailing byte clears, which is where the original puts those eight
 * load pairs. The other exact lifts of this target read the same cell through
 * the same declaration. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. The exact lift
 * decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4BmagicMagic14403_801F0134
 * decrements the same record byte 0xB on the same countdown reaching zero. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared per-frame counter in main RAM, outside this overlay's image. The
 * shared map owns the address (config/targets/shared/symbols.txt) and Splat
 * binds it through this target's generated undefined_syms_auto.txt, so no
 * target row is needed. Its low bit is the per-frame phase that the reviewed
 * exact bmagic lifts read - incrementWorkByteAOnFrameParityThenAdd16ToWorkByte9
 * AndAdvanceWorkByte2At80BmagicMagic00603_801F020C and the twin named below -
 * and the global word is relocated here as %hi/%lo. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* @source 0x801EF738
 * @behavior Per-frame handler of the scratchpad work object, entry 10 of the
 *           fifteen non-zero words of the in-image handler-pointer run based at
 *           0x801F04D0: the step dispatcher at 0x801EEE9C reads work byte 1,
 *           scales it by four and jalr's that entry, and the word at 0x801F04F8
 *           holds this address and is the only word in the payload that does,
 *           so no jal names this body. It first subtracts 2 from the work
 *           object's byte 9 at byte width (lbu / addiu -2 / sb) through the
 *           single read of the pointer cell at 0x1F800044. It then tests bit 0
 *           of the shared frame counter at 0x80143E6C (lui %hi / lw %lo /
 *           andi 0x1) and, when that bit is set, re-reads the pointer cell and
 *           increments byte 0xA by one. Finally it re-reads the cell and the
 *           byte it stored earlier, not the stored register - the intermediate
 *           byte-0xA store may replace the pointer itself - and compares that
 *           reloaded byte against 1, which the original materializes as addiu
 *           $v0,$zero,1. Only on equality does it read the shared battle work
 *           record pointer at 0x801EB4E0, decrement that record's byte 0xB by
 *           one at byte width, and clear work bytes 0x00 through 0x04, each
 *           clear re-reading the pointer cell so the original shows five
 *           separate load pairs. Leaf with no frame and no saved registers:
 *           takes no arguments, returns nothing through the jr $ra at
 *           0x801EF810 and reads or writes no other state. The 224-byte span is
 *           byte-identical to the reviewed exact lift
 *           decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrement
 *           GlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic09203_801EFA24
 *           (0x801EFA24, target emi/bmagic/magic092/03, payload offset 0xE24),
 *           proven with one payload comparison over all 396 out/binaries
 *           payloads, and that twin supplied this statement order, these
 *           operand forms and this name; it recorded this span as its own
 *           unlifted twin before this lift. The roles of work bytes 9, 0xA and
 *           0x00..0x04 and of record byte 0xB are not established, so they stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ByTwoIncrementWorkByteAOnFrameParityThenDecrementGlobalWorkByteBAndClearWorkBytes0To4AtOneBmagicMagic14403_801EF738(void) {
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
