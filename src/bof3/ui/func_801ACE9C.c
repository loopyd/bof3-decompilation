#include "bof3/ui/game00_internal.h"

/* @source 0x801ACE9C
 * @behavior returns the u16 entry at index from the stride-8 local table
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u16 func_801ACE9C(s32 arg0) {
  return *(u16*)&D_801459F4[(u16)arg0 * 8];
}
