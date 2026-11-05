#include "bof3/ui/shop00_internal.h"

/* @source 0x801D3828
 * @behavior writes the byte argument arg2 into one of the six consecutive
 *           bytes 0x0E..0x13 of the 164-byte field record selected from
 *           D_80144968 by the byte argument arg0, dispatching on the byte
 *           argument arg1: slot 0 keeps record byte 0x0E, slots 1..3 keep
 *           0x0F..0x11 and slots 4..5 keep 0x12..0x13, and the unsigned bound
 *           6 that guards the dispatch sends every larger arg1 to the slot 0
 *           arm as well. Each arm first calls the main-exe slot setter
 *           func_801665A0 with that arm's helper group (1 for slot 0, 2 for
 *           slots 1..3 and 3 for slots 4..5) and the new value, then calls the
 *           main-exe gate func_801650B4 with the same group and the record byte
 *           that the arm has not replaced yet, and only then stores the new
 *           value; both helper results are discarded.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFieldRecordSlotValue(u8 recordIndex, u8 slot, u8 value) {
  u8* record = &D_80144968[recordIndex * 164];

  switch (slot) {
  case 0:
  default:
    func_801665A0(1, value, 1, 0);
    func_801650B4(1, record[0x0E], 1, 0);
    record[0x0E] = value;
    break;
  case 1:
    func_801665A0(2, value, 1, 0);
    func_801650B4(2, record[0x0F], 1, 0);
    record[0x0F] = value;
    break;
  case 2:
    func_801665A0(2, value, 1, 0);
    func_801650B4(2, record[0x10], 1, 0);
    record[0x10] = value;
    break;
  case 3:
    func_801665A0(2, value, 1, 0);
    func_801650B4(2, record[0x11], 1, 0);
    record[0x11] = value;
    break;
  case 4:
    func_801665A0(3, value, 1, 0);
    func_801650B4(3, record[0x12], 1, 0);
    record[0x12] = value;
    break;
  case 5:
    func_801665A0(3, value, 1, 0);
    func_801650B4(3, record[0x13], 1, 0);
    record[0x13] = value;
    break;
  }
}
