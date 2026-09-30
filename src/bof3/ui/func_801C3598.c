#include "bof3/ui/game00_internal.h"

/* @behavior While the input record selected by work byte 0x13C carries flag bit
 * 5 in its D_80144974 flags word, decrements the input hold-delay byte at
 * work+0x119 and, when that byte reaches zero, clears bit 5 in the same indexed
 * record's flags word.
 * @source 0x801C3598
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit is instruction- and byte-exact: 45/45 instructions, 180 bytes.
 * The first seed reloaded the work pointer into the local (`work = D_80146250;`)
 * before reading byte 0x119, which put the reloaded pointer in `$a0` (register
 * reuse via the block-local first pointer) and left the reload in `$a0` where
 * the original has `$v1`; indexing the global directly at the reload points
 * (`D_80146250[0x119]` / `D_80146250[0x13C]`) allocates that cross-block value
 * to `$v1` and closes the last four instructions.
 */
void func_801C3598(void)
{
  u8* work;
  u8 index;

  work = D_80146250;
  index = work[0x13C];
  if ((D_80144974[index].flags & 0x20) == 0) {
    return;
  }

  work[0x119]--;
  if (D_80146250[0x119] != 0) {
    return;
  }

  index = D_80146250[0x13C];
  D_80144974[index].flags &= 0xFFDF;
}
