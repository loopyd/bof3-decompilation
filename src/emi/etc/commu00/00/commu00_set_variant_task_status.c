#include "internal.h"

/* @source 0x801F1204
 * @behavior sets scratch slot 6, stores variant-derived u16 to task slot entry
 */
void commu00_set_variant_task_status(u8 task_index, u8 record_kind_index) {
  volatile s8* ptr;
  u32          offset;

  ptr = SPAD_PTR_SLOT(volatile s8, 0x44);
  ptr[6] = 1;

  offset = (task_index & 0xFF) * 76;
  FIELD_REF(u16, commu00_task_label_words, offset * 2) =
      (u16)(commu00_variant_rotation[record_kind_index & 0xFF] - 0x3FFB);
}
