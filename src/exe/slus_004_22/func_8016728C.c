#include "internal.h"

/* possible name: emi_family_slot
 * @behavior maps a BOF3 content family plus index into the slot id used by the EXE
 * loader path.
 * @source 0x8016728C
 */
void func_8016728C(u8 index, u8 family) {
  u32 new_var;
  u32 slot_id;

  if (family != 3u) {
    EMI_STREAM_INDEX_HINT = index;
    if (family == 0u) {
      EMI_STREAM_INDEX_HINT = index | 0x80u;
    }
  }

  new_var = 0x27du;

  switch (family) {
    case 1:
      slot_id = (u32)index;
      slot_id = slot_id + 0x1dbu;
      break;
    case 0:
      slot_id = 0x26au;
      slot_id = (u32)index + slot_id;
      break;
    case 2:
      slot_id = (u32)index + 0x1eeu;
      break;
    case 3:
      slot_id = (u32)index + new_var;
      break;
    default:
      return;
  }

  func_80161FDC(slot_id);
}
