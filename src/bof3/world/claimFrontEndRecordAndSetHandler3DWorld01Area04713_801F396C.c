#include "bof3/bof3.h"

/* Shared free-record selector outside this payload. The body only calls it, so
 * the generated undefined_funcs_auto.txt row keeps the address-anchored name. */
s32 func_8019601C(void);

/* Base of the shared 0x74-byte front-end record table outside this payload; the
 * body loads it as a constant address, so the generated undefined_syms_auto.txt
 * row covers it and no map row is owed. */
extern u8 D_80143FC8[];

/* @source 0x801F396C
 * @behavior Asks the shared free-record selector func_8019601C for an index,
 * masks the result to a byte and, unless that byte is 0xFF, marks the selected
 * 0x74-byte front-end record at 0x80143FC8 occupied (byte +0 = 1) and writes
 * handler selector 0x3D into record byte +5. Takes no arguments and returns
 * nothing; the only in-image reference is the second word of the code-pointer
 * group at 0x801F3E5C, which holds 0x801F38F4, this address and 0x801F3A08.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordAndSetHandler3DWorld01Area04713_801F396C(void) {
  u8* record;
  s32 index;

  index = func_8019601C() & 0xFF;
  if (index != 0xFF) {
    record = (u8*)(D_80143FC8 + index * 0x74);
    record[0] = 1;
    record[5] = 0x3D;
  }
}
