#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The compiled original reaches the cell through a map symbol
 * (lui $v1,0x1F80 / lw $v1,0x44($v1)) rather than through a constant
 * address. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer outside this overlay, already resolved by the
 * generated undefined-symbol map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF410
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares its byte at offset 0xB against two with an unsigned bound
 *           (lbu/sltiu), so the guard is taken for every value below two. Only
 *           on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 once and increments the byte at offset 2 of that work
 *           object modulo 256 with lbu/addiu/sb, sharing the loaded base for
 *           the read and the store. Takes no arguments, returns nothing and
 *           keeps no frame, so the scratchpad cell is dereferenced only when
 *           the guard succeeds. Word 3 of the nine-word handler table based at
 *           0x801F08B0 names this address, so it runs as a battle step handler
 *           and the payload holds no jal caller for it. Bytes
 *           0x801EF410..0x801EF453 are byte-identical to the exact lift
 *           0x801EF5EC of bmagic/magic088/03, whose source is copied here with
 *           the scratchpad cell spelled by this target's own evidenced name
 *           and with only the symbol and file name changed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic16703_801EF410(void) {
  if (D_801EB4E0[0xB] < 2u) {
    g_battle_work[2]++;
  }
}
