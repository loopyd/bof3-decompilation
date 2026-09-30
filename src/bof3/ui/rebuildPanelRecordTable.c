#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7208
 * @behavior rebuilds the three 0x1C-byte records of the panel record table
 *           D_801E6098: the +0x15 slot byte of each record is first cleared to
 *           0xFF, then for every record index i of 0..2 the 0x18-byte-stride
 *           main-RAM table D_8018E990 is scanned in order and the first entry
 *           whose byte at +0x11 minus 0x30 equals i is passed to
 *           func_801EEC30 together with the constants 1, 1, 0x1C and 0xEA0,
 *           after which the seven 32-bit words held at the fixed main-RAM
 *           address 0x800C26A0 are copied into that record and the matching
 *           entry index is stored into the record's +0x15 slot byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
typedef struct ShopPanelWords {
  u32 w[7];
} ShopPanelWords;

void rebuildPanelRecordTable(void) {
  u8 i;
  u8 j;
  u8* slot;
  u8* record;

  /* The three slot bytes are the +0x15 field of the records. The original
   * materialises the record-0 slot address and derives the record base back
   * from it by that field offset, keeping the address in a scratch register
   * that also serves the record-0 store; taking the record base directly folds
   * the derivation and the store address into a single lui/addiu pair. The
   * three stores then follow the records in descending offset order, with only
   * the record-0 store reading the base register. */
  slot = &D_801E60AD;
  record = slot - 0x15;
  slot[0x38] = 0xFF;
  slot[0x1C] = 0xFF;
  slot[0] = 0xFF;
  for (i = 0; i < 3; i++) {
    /* The entry count is not cached: the original re-reads it for the inner
     * loop test of every record. */
    for (j = 0; j < D_801E6078; j++) {
      if (D_8018E990[j].unk_11 - 0x30 == i) {
        func_801EEC30(1, &D_8018E990[j], 1, 0x1C, 0xEA0);
        /* One 0x1C-byte aggregate copy: the original batches three loads then
         * three stores, which a per-word copy (seven assignments) does not
         * reproduce because it re-materialises the constant source base for
         * every word. */
        *PSX_PTR(ShopPanelWords, record) = *PSX_PTR(ShopPanelWords, 0x800C26A0u);
        record[0x15] = j;
      }
    }
    record += 0x1C;
  }
}
