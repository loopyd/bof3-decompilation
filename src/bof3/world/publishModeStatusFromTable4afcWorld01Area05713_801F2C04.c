#include "bof3/bof3.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */
extern u16 D_801F4AFC[]; /* @source 0x801F4AFC @kind table */

/* @source 0x801F2C04
 * @behavior Selects the halfword indexed by the signed shared mode byte
 *           D_801490C7 from the target-local halfword table at 0x801F4AFC and
 *           publishes it into the shared status halfword D_801490A8. Takes no
 *           arguments and returns nothing.
 *
 *           Constant and store evidence from the original bytes: the mode cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte, followed by a
 *           delay nop), the table base 0x801F4AFC is indexed by `sll` by one
 *           and `addu` into $at with `lhu`, and the status cell 0x801490A8
 *           takes the `sh`. The epilogue is a bare `jr $ra` with a delay `nop`,
 *           so the 0x2C-byte span is one complete leaf with a single return
 *           path and no argument registers live at entry.
 *
 *           Shape: the lifted sibling publishModeStatusFromTable362cWorld00Area
 *           01013_801F2C84 carries the same two-statement body - one `s32` mode
 *           temporary, the mode cell as the table index, the status halfword as
 *           the only store - at the same address in its own overlay. The two
 *           0x2C-byte payload spans compare equal except the `lhu` word, which
 *           holds each overlay's own `%lo` (0x362C there, 0x4AFC here), so this
 *           range is not byte-identical to that lifted span and the literals
 *           are written from this span's bytes rather than cloned. Siblings
 *           publishModeStatusFromTable33b4World00Area02113_801F2C6C and
 *           publishModeStatusFromTable3ad4World00Area00513_801F2C84 differ from
 *           this span in that same `lhu` word alone.
 *
 *           The table base sits in this payload's own data tail, and a pointer
 *           word at payload offset 0x16AC names this entry, so the caller is the
 *           script blob rather than a `jal` site and the parameter roles stay
 *           structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable4afcWorld01Area05713_801F2C04(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F4AFC[mode];
}
