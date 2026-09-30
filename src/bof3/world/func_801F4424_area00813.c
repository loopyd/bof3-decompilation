#include "bof3/world/area00813_internal.h"

/* @source 0x801F4424
 * @behavior claims the front-end record slot selected by func_8019601C,
 *           publishing that index in the scratchpad byte 0x1F800000; when the
 *           index is not 0xFF it marks the record's occupancy byte, sets the
 *           record byte 0x05 to 0x54 and stores the current work-entity index
 *           (the D_80146884 minus D_80146888 pointer difference) in record byte
 *           0x0B.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F4424(void) {
  u8* scratch;
  u8 index;

  index = func_8019601C();
  scratch = &D_1F800000;
  *scratch = index;
  if (index != 0xFF) {
    D_80143FC8[*scratch].flags_00 = 1;
    D_80143FC8[*scratch].unk_05 = 0x54;
    D_80143FC8[*scratch].unk_0B = (u8)(D_80146884 - D_80146888);
  }
}
