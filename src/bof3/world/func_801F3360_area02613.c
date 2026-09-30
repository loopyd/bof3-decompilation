#include "bof3/world/area02613_internal.h"

/* @source 0x801F3360
 * @behavior Claims the next free front-end record from the allocator
 *           func_8019601C and, when the returned index is not 0xFF, marks the
 *           0x74-byte record it selects at 0x80143FC8 as occupied (byte 0x00
 *           set to 1), writes 0x33 into its byte 0x05, stores 0x70000 and
 *           0x68000 in its words 0x34/0x38, and stores the distance returned
 *           by func_8015477C for that word pair, scaled by 0x10000, in its word
 *           0x3C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3360(void) {
  World00Area026Record* record;
  u8 index;

  index = func_8019601C();
  if (index != 0xFF) {
    record = &D_80143FC8[index];
    record->flags_00 = 1;
    record->unk_05 = 0x33;
    record->unk_34 = 0x70000;
    record->unk_38 = 0x68000;
    record->unk_3C = func_8015477C(record->unk_34, 0x68000) << 16;
  }
}
