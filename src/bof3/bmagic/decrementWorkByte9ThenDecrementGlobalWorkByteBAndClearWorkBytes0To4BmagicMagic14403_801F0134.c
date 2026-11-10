#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, bound at 0x1F800044 by this target's map
 * row g_battle_work. A byte store through the loaded pointer may alias the
 * cell, so the compiler re-materializes the address (lui $vN,0x1F80 / lw
 * $vN,68($vN)) once for the byte-9 countdown and once for each of the five
 * trailing byte clears, which is where the original puts those six load pairs.
 * The two other exact lifts of this target read the same cell through the same
 * declaration. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row or WEAK_SYMBOL_AT binding is
 * needed. The exact lift
 * incrementWorkByte2WhenGlobalWorkByteBFlag80SetBmagicMagic14403_801F00F0
 * guards on the same record byte 0xB. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0134
 * @behavior Per-frame handler of the scratchpad work object that only the
 *           in-image handler-pointer run based at 0x801F04D0 names: the word at
 *           0x801F0508, entry 14 of the fifteen non-zero words of that run, is
 *           the single word in this payload that holds this address, and no jal
 *           in the payload targets it, so the dispatcher that reads the run
 *           sits outside this overlay's image. It decrements the work object's
 *           unsigned byte 9 in place through the pointer cell at 0x1F800044
 *           (lbu / addiu -1 / sb) and tests the stored byte truncated to eight
 *           bits (andi / bnez) rather than a reload, so the rest of the body
 *           runs only when that countdown reaches zero. On zero it reads the
 *           shared battle work record pointer at 0x801EB4E0, decrements that
 *           record's byte 0xB by one at byte width, and then clears work bytes
 *           0x00 through 0x04, each clear re-reading the pointer cell so the
 *           original shows five separate load pairs. Leaf with no frame and no
 *           saved registers: takes no arguments and returns nothing through the
 *           jr $ra at 0x801F01C8, and reads or writes no other state. The roles
 *           of work bytes 9 and 0..4 and of record byte 0xB are not
 *           established, so they stay offset-anchored. The 156-byte span is
 *           byte-identical to the reviewed exact lift
 *           decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4
 *           BmagicMagic09203_801F0420 (0x801F0420, target emi/bmagic/magic092/03,
 *           payload offset 0x1820), which supplied this statement order and
 *           name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenDecrementGlobalWorkByteBAndClearWorkBytes0To4BmagicMagic14403_801F0134(void) {
  if (--g_battle_work[0x09] == 0) {
    D_801EB4E0[0x0B]--;
    g_battle_work[0x00] = 0;
    g_battle_work[0x01] = 0;
    g_battle_work[0x02] = 0;
    g_battle_work[0x03] = 0;
    g_battle_work[0x04] = 0;
  }
}
