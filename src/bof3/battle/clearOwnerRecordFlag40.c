#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC844
 * @behavior Clears bit 0x40 in the leading byte of the battle record selected
 * by the owner id.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the u8 parameter's zero-extension lands in the findOwnerRecord
 * `jal` delay slot and the returned pointer stays in $v0, so the record store
 * needs no address temporary.
 */
void clearOwnerRecordFlag40(u8 id)
{
  u8 *record;

  record = findOwnerRecord(id);
  record[0] &= 0xBF;
}
