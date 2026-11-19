#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F3808[]; /* @source 0x801F3808 @kind table */

/* @source 0x801F2CB0
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F3808 and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments, calls nothing and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte, followed by a
 *           load-delay `nop`), the table base 0x801F3808 is indexed by `sll` by
 *           one and `addu` into $at with `lhu`, and the status cell 0x801490A8
 *           takes the `sh`. The epilogue is a bare `jr $ra` with a delay `nop`,
 *           so the 0x2C-byte span is one complete leaf with a single return
 *           path and no argument registers live at entry. The table itself is
 *           payload data at 0x801F3808: three halfwords 0x2A, 0x2B and 0x2C at
 *           the end of this overlay. The body does not bound the signed index;
 *           those three entries alone do not prove the runtime index domain.
 *
 *           Shape: publishModeStatusFromTable3634World00Area01013_801F2CB0,
 *           lifted exact at this same address in its own overlay, carries the
 *           same two-statement body - one `s32` mode temporary, the mode cell
 *           as the table index, the status halfword as the only store. A byte
 *           comparison of the two 0x2C-byte payload spans differs in one word,
 *           the `lhu` displacement (0x3634 there, 0x3808 here), so this span
 *           has no byte-identical twin and the table literal is written from
 *           these bytes rather than cloned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable3808World03Area15013_801F2CB0(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F3808[mode];
}
