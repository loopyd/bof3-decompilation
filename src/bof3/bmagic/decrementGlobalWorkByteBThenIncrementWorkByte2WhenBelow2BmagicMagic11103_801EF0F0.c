#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds 0x1F800044 to
 * this name, so each access is a lui $vN,0x1F80 / lw $vN,0x44($vN) pair. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * keeps its row in the generated undefined-symbol map because the overlay's
 * remaining assembly references the same address. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF0F0
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares its record byte 0xB against two with an unsigned bound
 *           (lbu/sltiu), so the guard is taken for every value below two. Only
 *           on that path it stores byte 0xB minus one back at byte width
 *           (addiu -1/sb), then reads the scratchpad work-object pointer cell
 *           at 0x1F800044 once and increments the work object's byte 0x2 with
 *           a byte-width add (lbu/addiu 1/sb), advancing the sub-step index
 *           that this overlay's dispatcher indexes on. Shares its
 *           guard-and-increment tail with the exact sibling
 *           incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic06203_801EF5D8,
 *           which tests the same record byte against three and adds no store.
 *           Takes no arguments, returns nothing and keeps no frame, so the
 *           scratchpad cell is dereferenced only when the guard succeeds.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBThenIncrementWorkByte2WhenBelow2BmagicMagic11103_801EF0F0(
    void) {
  if (D_801EB4E0[0xB] < 2u) {
    D_801EB4E0[0xB]--;
    g_battle_work[2]++;
  }
}
