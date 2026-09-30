#include "bof3/bof3.h"

/* Shared halfword read by the sibling battle lifts as the selected battle kind
 * (src/bof3/battle/applyKindMaskSelection.c reads it as `index`,
 * src/bof3/battle/resolveKindResultMode.c as `state`/`selected_kind`). */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* @source 0x801EEF3C
 * @behavior Reads the shared battle-kind halfword at 0x801463C0 and returns the
 * result code 2 when it equals 0x27, 3 when it equals 0xA3, and 2 for any other
 * value. The original gives both 2-valued outcomes one shared return block, so
 * the two tests jump to the label holding their code and fall through to the
 * shared one; the caller stores the byte in work byte 0x04.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 resolveKindResultCodeBmagicMagic03903_801EEF3C(void) {
  u16 kind = D_801463C0;

  if (kind == 0x27) {
    goto kind_27;
  }
  if (kind == 0xA3) {
    goto kind_a3;
  }
kind_27:
  return 2;
kind_a3:
  return 3;
}
