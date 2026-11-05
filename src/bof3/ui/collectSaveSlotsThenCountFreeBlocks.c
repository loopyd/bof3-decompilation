#include "bof3/ui/shop00_internal.h"

/* @source 0x801D6F00
 * @behavior rebuilds the save-slot list of this overlay: it fills the 0x18-byte
 *           entry table D_8018E990 from the main-exe save-directory reader
 *           func_801EFCA8 with the wildcard pattern D_801D0C94 and the 0x10
 *           entry limit, stores the returned entry count into D_801E6074 and
 *           clears the kept-entry cursor D_801E610C. It then walks the entries
 *           in order and, for every entry whose first 0x10 bytes equal the
 *           game-name prefix D_801D0C20, copies the whole 0x18-byte entry to the
 *           cursor position and increments that cursor, compacting the matching
 *           entries to the front of the table. Every entry also contributes to a
 *           used-block byte: the bits above 13 of its +0x14 word plus one when
 *           its low 13 bits are nonzero, which accounts for a partially used
 *           block. The step publishes the used-block count subtracted from 0xF
 *           into D_801E60FC (remaining capacity of the list) and the kept-entry
 *           count into D_801E6078 (the slot count of the shop panel records and
 *           the cursor bound of the list navigation).
 * @status exact
 * @match 100.00
 * @residual none
 */
void collectSaveSlotsThenCountFreeBlocks(void) {
  u8 count;
  u8 i;
  u8 used;
  u32 entry;

  count = func_801EFCA8(D_801D0C94, 0x10);
  used = 0;
  D_801E6074 = count;
  D_801E610C = 0;
  for (i = 0; i < D_801E6074; i++) {
    entry = D_8018E990[i].unk_14;
    /* The bound test is grouped with the entry's own block count and the running
     * total is the left addend of the whole sum: the original evaluates that
     * inner (bound + blocks) pair first, which puts the block count in a0 and
     * the masked total in v1, and joins with the total-derived value as the
     * first operand. Grouping the bound test with the total instead moves that
     * register pair, and putting the inner pair on the left of the total swaps
     * the operand order of both joins. */
    used = used + (((entry & 0x1FFF) != 0 ? 1 : 0) + (entry >> 13));
    if (strncmp((char*)&D_8018E990[i], D_801D0C20, 0x10) == 0) {
      D_8018E990[D_801E610C] = D_8018E990[i];
      D_801E610C = D_801E610C + 1;
    }
  }
  D_801E60FC = 15 - used;
  D_801E6078 = D_801E610C;
}
