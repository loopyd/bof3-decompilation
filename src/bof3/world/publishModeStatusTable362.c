#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F362C[]; /* @source 0x801F362C @kind table */

/* @source 0x801F2C84
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F362C and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte, followed by a
 *           delay nop), the table base 0x801F362C is indexed by `sll` by one
 *           and `addu` into $at with `lhu`, and the status cell 0x801490A8
 *           takes the `sh`. The epilogue is a bare `jr $ra` with a delay `nop`,
 *           so the 0x2C-byte span is one complete leaf with a single return
 *           path and no argument registers live at entry.
 *
 *           Shape: the exact sibling publishModeStatusFromTable3ad4World00Area
 *           00513_801F2C84 carries the same two-statement body - one `s32` mode
 *           temporary, the mode cell as the table index, the status halfword as
 *           the only store - at the same address in its own overlay. The two
 *           0x2C-byte payload spans compare equal except the `lhu` word, which
 *           holds each overlay's own `%lo` (0x3AD4 there, 0x362C here), so this
 *           range is not byte-identical to that lifted span and the literals
 *           are written from this span's bytes rather than cloned. This
 *           target's own neighbouring sibling
 *           publishModeStatusFromTable3634World00Area01013_801F2CB0 has the
 *           same body and reads the neighbouring halfword table at 0x801F3634,
 *           eight bytes past this one; its original-byte disassembly
 *           independently confirms the statement order and the table-pair
 *           layout.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusTable362(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F362C[mode];
}
