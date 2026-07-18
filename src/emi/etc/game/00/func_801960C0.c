#include "internal.h"

/* @behavior clears the first five bytes of the record slot at the given
 * index, zeroing per‑slot fields in the entry table at D_80143FC8.
 * @source 0x801960C0
 */
void func_801960C0(u8 record_index) {
  D_80143FC8[record_index].flags_00 = 0;
  D_80143FC8[record_index].unk_01 = 0;
  D_80143FC8[record_index].unk_02 = 0;
  D_80143FC8[record_index].unk_03 = 0;
  D_80143FC8[record_index].unk_04 = 0;
}
