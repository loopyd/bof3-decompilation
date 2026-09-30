#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC934
 * @behavior Clears the five leading bytes of the battle record selected by the
 * owner id.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the u8 parameter's zero-extension lands in the findOwnerRecord
 * `jal` delay slot and the returned pointer stays in $v0, so each clear is a
 * direct `sb $zero` with no address temporary (same shape as
 * clearOwnerRecordFlag40); the last store fills the `jr` delay slot after the
 * $ra restore.
 */
void clearOwnerRecordHead(u8 id)
{
  u8 *record;

  record = findOwnerRecord(id);
  record[0] = 0;
  record[1] = 0;
  record[2] = 0;
  record[3] = 0;
  record[4] = 0;
}
