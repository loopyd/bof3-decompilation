#include "bof3/bof3.h"

/* Shared battle-kind halfword at 0x801463C0. The sibling battle lifts read it
 * as the selected battle kind (resolveKindResultCodeBmagicMagic03903_801EEF3C
 * consumes the same halfword); it lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* @source 0x801F0230
 * @behavior Reads the shared battle-kind halfword at 0x801463C0 and returns the
 * result code 7 when it equals 7 and 0xFF for any other value; the only caller,
 * func_801EFD90, stores that byte in work byte 0x04 of the scratchpad work
 * object, the same slot the sibling kind resolver feeds. The original
 * materializes the selected code in $a0 and copies it to $v0 at the single
 * shared return.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 resolveKindResultCodeBmagicMagic00800_801F0230(void) {
  u8 code = 0xFF;

  if (D_801463C0 == 7) {
    code = 7;
  }
  return code;
}
