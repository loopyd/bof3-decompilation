#include "bof3/bof3.h"

extern s8 D_801490C7;    /* @source 0x801490C7 @kind data */
extern u16 D_801490A8;   /* @source 0x801490A8 @kind data */
extern u16 D_801F5CD8[]; /* @source 0x801F5CD8 @kind table */

/* @source 0x801F2C54
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F5CD8 and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte), the table
 *           base 0x801F5CD8 is indexed by `sll` by one and `addu` into $at with
 *           `lhu`, and the status cell 0x801490A8 takes the `sh`. The epilogue
 *           is a bare `jr $ra` with a delay `nop`, so the 0x2C-byte span is one
 *           complete leaf with a single return path and no argument registers
 *           live at entry.
 *
 *           Shape: the same-target asm span at 0x801F2C04 carries the same
 *           eight-instruction prefix, publishing the neighbouring halfword
 *           table 0x801F5CD4 through the same mode cell and status cell before
 *           its gate test. Each of the 24 payloads under out/binaries that
 *           carries this exact shape holds its own table displacement, and the
 *           `lhu` word here (0x94225CD8) matches none of them, so this span has
 *           no byte-identical twin and the table literal is written from these
 *           bytes rather than cloned. The corpus sibling at 0x801F2CB0 in
 *           emi/world00/area005/13 publishes its own halfword table through the
 *           same mode and status cells with this statement order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable5(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F5CD8[mode];
}
