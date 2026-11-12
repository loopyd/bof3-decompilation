#include "bof3/ui/shop00_internal.h"

/* @source 0x801D68D0
 * @behavior field-record rebuild of the shop front end, called by the phase
 *           step func_801D6668 with the constant 0 and by the UI sub-step
 *           handler emitPanelStripThenRebuildShopStateOnClearGate with the
 *           constant 1: it walks the eight 164-byte field records of the
 *           main-RAM table D_80144968 and, when its byte argument is nonzero,
 *           first clears the record byte at +0x1A, submits the field request
 *           func_801656EC with the record index, 0xA0 and 0, and copies the
 *           byte at +0x2A into +0x18. For every record it then clears the
 *           bit-mask byte at +0x19, rebuilds the record through func_80164A44,
 *           clears +0x19 a second time, restores the halfwords at +0x14 and
 *           +0x16 from the pair at +0x1C and +0x1E, and submits
 *           func_801656EC with the record index, 0x2000 and 0. After the walk
 *           it tests the master flag bank: when the flag helper func_8015B5D4
 *           reports flag 4 set at the address 0x8014554C and clear at
 *           0x8014554F, and the main-RAM byte 0x80145513 is below 0x1E, that
 *           byte is cleared; the three cells sit 0xBE4, 0xBE7 and 0xB8B past
 *           the same table base, so the original reaches them through
 *           D_80144968 and they are spelled that way here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void rebuildFieldRecordSlots(u8 arg0) {
  u8* record;
  u8 index;
  u16 pair_14;
  u16 pair_16;

  index = 0;
  do {
    record = D_80144968 + index * 164;
    if (arg0 != 0) {
      record[0x1A] = 0;
      func_801656EC(index, 0xA0, 0);
      record[0x18] = record[0x2A];
    }
    record[0x19] = 0;
    func_80164A44(record);
    record[0x19] = 0;
    pair_14 = *(u16*)(record + 0x1C);
    pair_16 = *(u16*)(record + 0x1E);
    *(u16*)(record + 0x14) = pair_14;
    *(u16*)(record + 0x16) = pair_16;
    func_801656EC(index, 0x2000, 0);
    index += 1;
  } while (index < 8);
  if (func_8015B5D4((u32)(D_80144968 + 0xBE4), 4) != 0) {
    if (func_8015B5D4((u32)(D_80144968 + 0xBE7), 4) == 0) {
      if (D_80144968[0xB8B] < 0x1E) {
        D_80144968[0xB8B] = 0;
      }
    }
  }
}
