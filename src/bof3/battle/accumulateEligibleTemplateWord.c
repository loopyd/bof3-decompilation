#include "bof3/battle/battle03_internal.h"

/* @source 0x801DD59C
 * @behavior Adds the argument to the third word of the 164-byte template
 * record at 0x80144968 selected by the mapped local-work byte +0x79 of every
 * eligible active local work record below the volatile count at 0x801462F0,
 * saturating that word at 0x98967F, and returns the argument.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 accumulateEligibleTemplateWord(s32 arg0) {
  u8 index = 0;
  u8 record;

  if (D_801462F0 != 0) {
    do {
      if ((u8)testFlag400WhenEligible(index) != 0) {
        record = lookupStateByteMapped(D_80145E90[index].unk_79);
        if (D_80144968[record].words_00[2] + arg0 <= 0x98967Eu) {
          record = lookupStateByteMapped(D_80145E90[index].unk_79);
          D_80144968[record].words_00[2] += arg0;
        } else {
          record = lookupStateByteMapped(D_80145E90[index].unk_79);
          D_80144968[record].words_00[2] = 0x98967Fu;
        }
      }
      index += 1;
    } while (index < D_801462F0);
  }
  return arg0;
}
