#include "bof3/bof3.h"

/* The overlay's battle-magic record pool. The frame loop at 0x801EEC28
 * publishes 0x801F01B0 + i * 0x2C into the pointer cell at 0x801F17B0, and this
 * function hands out one pool slot index at a time. 128 records of 0x2C bytes
 * span 0x801F01B0 through 0x801F17AF, ending exactly at that pointer cell. The
 * trailing data block carries no map row, so the reference binds through the
 * target's generated undefined-symbol table. */
extern u8 magic16203RecordFlags[]; /* @source 0x801F01B0 @kind data */

/* @source 0x801EFFFC
 * @behavior Reserves the first of 128 records in the pool at 0x801F01B0 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x2C. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 128 records are occupied. The caller at 0x801EEE6C
 *           compares the returned byte against 0xFF and then steps the
 *           reserved record by index * 0x2C to clear its bytes at offsets 0x01
 *           and 0x02, store the frame pointer at 0x28 and seed the byte at
 *           0x03, so a zero-extended byte result is what it consumes. Takes no
 *           arguments. The body matches the shape of
 *           reserveFirstFreeRecordBmagicMagic13103_801F1A58 up to the pool
 *           address, the slot count and the record stride.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic16203_801EFFFC(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x80; index++) {
        offset = index * 0x2C;
        flags = magic16203RecordFlags[offset];
        if (!(flags & 1)) {
            magic16203RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
