#include "bof3/world/area02713_internal.h"

/**
 * @source 0x801F3710
 * @behavior Claims the first free front-end record slot by marking its occupancy
 *           byte and storing 0x2F in the record's byte 0x05.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3710(void) {
  World00Area027Record* record;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[index];
    record->flags_00 = 1;
    record->unk_05 = 0x2F;
  }
}
