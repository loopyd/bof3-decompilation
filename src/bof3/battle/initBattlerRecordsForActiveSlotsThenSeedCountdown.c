#include "bof3/battle/battle15_internal.h"

extern void func_800ABCE0(u8 index, u8 identity, u32 word_04, u32 word_08); /* @source 0x800ABCE0 */

/* @source 0x800ABC30
 * @behavior Clears the initialised battler-record counter D_801462F2, walks the
 *   eight 12-byte slot records of D_801463DC and, for every slot whose leading
 *   byte is set, initialises the next battler record through func_800ABCE0 with
 *   that slot's identity byte and the word pair at 0x04 and 0x08, passing the
 *   current number of initialised records as the record index. After the walk it
 *   copies the counter D_801462F2 into D_801462F3, the countdown that
 *   clearBattlerFlagsAndDecrement462F3 decrements as each battler finishes.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Levers: analysis data-candidates reports no data span in this target, the span
 * opens with a stack prologue and closes with jr $ra, and a scan of every
 * shipped payload for these 0xB0 bytes finds only this payload, so the data,
 * carve and clone levers are declined and the body is seeded from its own
 * bytes. The slot table type follows the two game/00 passes that fill the same
 * records: 0x801C6AEC writes the active byte and the identity byte at 0x01 and
 * steps the base by 0xC, and 0x801C6EE0 reads the identity byte at 0x01 and the
 * word pair at 0x04 and 0x08.
 */
void initBattlerRecordsForActiveSlotsThenSeedCountdown(void) {
  u8 i;
  u8 count;

  i = 0;
  count = 0;
  D_801462F2 = 0;
  while (i < 8) {
    if (D_801463DC[i].active != 0) {
      func_800ABCE0(count, D_801463DC[i].identity, D_801463DC[i].word_04,
                    D_801463DC[i].word_08);
      count++;
    }
    i++;
  }
  D_801462F3 = D_801462F2;
}
