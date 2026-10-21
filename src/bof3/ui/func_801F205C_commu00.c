#include "bof3/ui/commu00_internal.h"

/* @source 0x801F205C
 * @behavior Reads the high nibble of the fairy slot's stride-8 active-record
 * state byte to pick the local 6-byte label record. While that record's gate
 * word is clear it selects the record's first label and, for level 0 only,
 * resets the fairy progress byte to 2 instead of advancing it. While the gate
 * word is set it copies the record's twelve-byte item name through the local
 * label helper, clears that record's state bytes when the helper reports the
 * record as consumed, selects one of the two record labels, and advances the
 * fairy progress byte. Every path starts the selected action through
 * func_80150224 and latches the shared phase byte to 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F205C(void) {
  u8 *name;
  u8 *slot;
  s16 label;
  u8 state;
  u8 level;
  s32 i;

  state = activeRecordBytes[fairySlotIndex].unk_03;
  level = state >> 4;

  if (D_801F26CC[level].unk_04 == 0) {
    label = D_801F26CC[level].unk_00;
    if (level == 0) {
      fairyProgress[0] = 2;
    } else {
      D_801448EC += 1;
    }
  } else {
    name = func_80165D48(state & 0xF,
                         activeRecordBytes[fairySlotIndex].record_state);
    for (i = 0; i < 12; i++) {
      D_801490D8[i] = name[i];
    }
    slot = &fairySlotIndex;
    D_801490E4[0] = 0;
    if (func_801650B4(activeRecordBytes[*slot].unk_03 & 0xF,
                      activeRecordBytes[*slot].record_state,
                      (u8)D_801F26CC[level].unk_04, 0) != 0) {
      activeRecordBytes[*slot].record_state = 0;
      activeRecordBytes[*slot].unk_03 = 0;
      do {
        label = D_801F26CC[level].unk_00;
      } while (0);
    } else {
      label = D_801F26CC[level].unk_02;
    }
    fairyProgress[0] += 1;
  }

  func_80150224(label);
  D_80143BB0 = 2;
}
