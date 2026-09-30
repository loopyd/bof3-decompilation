#include "bof3/bof3.h"

/* Target-local declarations: this overlay has no private header, so every
 * extern it uses lives here. D_80145AA8 is the shared pad/input word (read
 * with lhu), D_80143FC8 is the base of the shared 0x74-byte field-object
 * record table and func_8019601C is the shared "current object index"
 * accessor whose low byte selects the record. */
extern u16 D_80145AA8;
extern u8 D_80143FC8[];
extern s32 func_8019601C();

/* @source 0x801F6CA4
 * @behavior On frames where the shared pad/input word D_80145AA8 has bit 3
 * (0x8) set it reads the currently selected field-object index from
 * func_8019601C() and, unless that index's low byte is 0xFF (nothing
 * selected), marks that object's 0x74-byte record in the shared
 * field-object record table at 0x80143FC8: record byte +0 = 1 (the record
 * becomes active) and record byte +5 = 0x2F (the record's kind/state code).
 * Both stores address the same record pointer, which is why the original
 * materialises the table base once and uses displacements 0 and 5. It takes
 * no arguments, returns nothing and reads or writes no other memory; the
 * index is masked to one byte because func_8019601C() also carries
 * unrelated status bits in its upper bytes.
 * @status unverified
 * @match 0.00
 * @residual none
 */
void markSelectedRecordActiveOnInputBit3(void) {
  u8 index;

  if (D_80145AA8 & 8) {
    index = func_8019601C() & 0xff;

    if (index != 0xffu) {
      D_80143FC8[(u32)index * 0x74u] = 1u;
      D_80143FC8[(u32)index * 0x74u + 5] = 0x2fu;
    }
  }
}
