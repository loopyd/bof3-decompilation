#include "bof3/bof3.h"

/* The overlay's battle-magic record pool. The frame loop at 0x801EEC28
 * publishes 0x801F01B0 + i * 0x2C into the pointer cell at 0x801F17B0 and this
 * function hands out one pool slot index at a time. 128 records of 0x2C bytes
 * span 0x801F01B0 through 0x801F17AF, ending exactly at that pointer cell. */
extern u8 magic16803RecordFlags[]; /* @source 0x801F01B0 @kind data */

/* @source 0x801EFFFC
 * @behavior Reserves the first of 128 records in the pool at 0x801F01B0 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x2C. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 128 records are occupied. Its only caller is
 *           func_801EED38 at 0x801EEE6C, which compares the returned byte
 *           against 0xFF and then steps the reserved record by index * 0x2C to
 *           clear its bytes at offsets 0x01 and 0x02, store the scratchpad
 *           work pointer at 0x28 and seed the byte at 0x03 with rand() & 3, so
 *           a zero-extended byte result is what it consumes. No payload word
 *           holds this entry address; the jal is the only reference. Takes no
 *           arguments. The 108-byte body was byte-identical to
 *           emi/bmagic/magic162/03 0x801EFFFC, which sits at the same payload
 *           offset under the same load address and measured exact, and it holds
 *           the shape of the family's other reserving lifts up to the pool
 *           address, the slot count and the record stride.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic16803_801EFFFC(void) {
  u8 index;
  u8 flags;
  u32 offset;

  for (index = 0; index < 0x80; index++) {
    offset = index * 0x2C;
    flags = magic16803RecordFlags[offset];
    if (!(flags & 1)) {
      magic16803RecordFlags[offset] = flags | 1;
      return index;
    }
  }
  return 0xFF;
}
