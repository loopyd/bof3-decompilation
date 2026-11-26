#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * overlay's remaining assembly still reads the record through the same
 * address, so the generated undefined-symbol map of this target keeps the row
 * D_801EB4E0 = 0x801EB4E0 and no target-map row is needed. The record is read
 * only through the byte view below. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. This target's map binds 0x1F800044 to
 * g_battle_work, and the original re-materialises the address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each access, so every use below takes
 * the named cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF870
 * @behavior Work-byte-2 handler: reads the shared battle work pointer at
 *           0x801EB4E0 once and compares that record's byte 0xB against three
 *           with an unsigned bound (lbu/sltiu), so the guarded path runs for
 *           every value below three. Only on that path it reads the scratchpad
 *           work-object pointer cell at 0x1F800044, stores zero into the work
 *           object's byte 9 (sb $zero), re-reads the cell and increments the
 *           object's byte 2 modulo 256 with a byte-width add
 *           (lbu/addiu 1/sb), advancing the sub-step index this overlay's
 *           dispatcher selects on. Leaf with no frame; takes no arguments,
 *           returns nothing and reads or writes no other state. The payload
 *           word at 0x801EFAB8 is the only in-image reference to this
 *           address: it is the first of the three-word work-byte-2 handler run
 *           at 0x801EFAB8 (0x801EF870, 0x801EF8C4, 0x801EF914), which
 *           0x801EF7F8 selects with work-object byte 2, and the same three
 *           words are entries 7 to 9 of the ten-word byte-1 table based at
 *           0x801EFA9C. No other payload under out/binaries held these 84
 *           bytes, so the statement order and operand forms came from exact
 *           lifts that carried the same regions: the unsigned guard and
 *           guarded byte-2 advance of
 *           incrementWorkByte2WhenGlobalWorkByteBBelow3BmagicMagic11003_801EF8F8,
 *           and the store-then-advance pair with the cell re-read between them
 *           of setWorkByte29Increment.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateAdvanceGlobalByte(
    void) {
  if (D_801EB4E0[0xB] < 3u) {
    g_battle_work[9] = 0;
    g_battle_work[2]++;
  }
}
