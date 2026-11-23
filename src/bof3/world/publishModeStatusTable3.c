#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F3AD4[]; /* @source 0x801F3AD4 @kind table */

/* @source 0x801F2C84
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F3AD4 and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte), the table
 *           base 0x801F3AD4 is indexed by `sll` by one and `addu` into $at
 *           with `lhu`, and the status cell 0x801490A8 takes the `sh`. The
 *           epilogue is a bare `jr $ra` with a delay `nop`, so the 0x2C-byte
 *           span is one complete leaf with a single return path and no
 *           argument registers live at entry.
 *
 *           Shape: the exact sibling publishModeStatusFromTable5390World01Area
 *           06714_801F3E50 carries the same two-statement body - one `s32` mode
 *           temporary, the mode cell as the table index, the status halfword as
 *           the only store - and the 13 payload spans in out/binaries whose
 *           `lui`/`lb`/`sll`/`addu`/`lhu` prefix matches this one differ in the
 *           `lhu` word alone, because each indexes its own overlay-local table.
 *           The 0x2C-byte range is therefore not byte-identical to any lifted
 *           span (the closest, this target's own 0x801F2CB0, reads the table at
 *           0x801F3ADC), so the literals are written from this span's bytes
 *           rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable3(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F3AD4[mode];
}
