#include "bof3/ui/game00_internal.h"

/* @source 0x801C1E88
 * @behavior counts the active front-end records claimed by an entry slot: walks
 * the eight stride-8 local record slots at D_801457A8/D_801457A9 and, for every
 * slot whose kind byte is 4 and whose argument byte equals arg0, walks the 60
 * stride-8 active-record entries at D_801455C8/D_801455C9, incrementing the
 * count for each entry whose presence byte is nonzero and whose owning slot
 * byte equals slot + 1; when no entry claims a matching slot the supplied
 * counter word is incremented.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countActiveRecordsForSlot(u8 arg0, s32* counter) {
  u8  count;
  s32 slot;
  s32 offset;
  s32 record;
  s32 owner;

  count = 0;
  for (slot = 0; slot < 8; slot++) {
    offset = slot * 8;
    if (D_801457A8[offset] == 4 && D_801457A9[offset] == arg0) {
      owner = slot + 1;
      for (record = 0; record < 0x1E0; record += 8) {
        if (D_801455C8[record] != 0 && D_801455C9[record] == owner) {
          count++;
        }
      }
    }
  }
  if (count == 0) {
    (*counter)++;
  }
}
