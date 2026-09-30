#include "bof3/ui/game00_internal.h"

/* @behavior resolves the current world state ID through func_8019A194 and
 * returns word 0 of the matching 0x1C-byte slot record in the eleven-entry
 * table at 0x801C7F70; that word is the slot's record pointer (the caller
 * dereferences it) and the record key byte sits at record+4.
 * @source 0x8019A1D4
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_8019A1D4(void)
{
  u8 index;

  index = func_8019A194();
  return D_801C7F70[index][0];
}
