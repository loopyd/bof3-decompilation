#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F3ADC[]; /* @source 0x801F3ADC @kind table */

/* @source 0x801F2CB0
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F3ADC and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte), the table
 *           base 0x801F3ADC is indexed by `sll` by one and `addu` into $at with
 *           `lhu`, and the status cell 0x801490A8 takes the `sh`. The epilogue
 *           is a bare `jr $ra` with a delay `nop`, so the 0x2C-byte span is one
 *           complete leaf with a single return path and no argument registers
 *           live at entry.
 *
 *           Shape: the same-target sibling at 0x801F2C84 publishes the
 *           neighbouring halfword table 0x801F3AD4 through the same mode cell
 *           and status cell. A byte comparison of the two 0x2C-byte spans in
 *           out/binaries/emi/world00/area005/13.bin differs at one byte, the low
 *           byte of the `lhu` displacement (0x3AD4 there, 0x3ADC here), so this
 *           span has no byte-identical twin and the table literal is written
 *           from these bytes rather than cloned. The payload pointer run at
 *           0x801F39E0 holds 0x801F2C04, 0x801F2C44, 0x801F2C84 and 0x801F2CB0
 *           in consecutive slots, so both publishers are entries of one
 *           overlay-local handler table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F3ADC[mode];
}
