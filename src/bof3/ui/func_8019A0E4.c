#include "bof3/ui/game00_internal.h"

/* @source 0x8019A0E4
 * @behavior Scans the 20 entry records at recordTable; for every record whose
 * flags_00 byte is nonzero it publishes the record address as the active work
 * area through the scratchpad pointer cell at 0x1F800044 and dispatches the
 * handler table at 0x801C7C70 indexed by the record byte at offset 0x05.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8019A0E4(void) {
  u8          record_index;
  RecordSlot* record;

  record_index = 0u;
  while (record_index < 20u) {
    if (recordTable[record_index].flags_00 != 0) {
      record = &recordTable[record_index];
      g_game_work = (struct GameWorkArea*)record;
      D_801C7C70[record->unk_05]();
    }
    record_index += 1u;
  }
}
