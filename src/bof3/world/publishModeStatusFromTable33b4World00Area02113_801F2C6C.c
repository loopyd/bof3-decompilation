#include "bof3/bof3.h"

/* Shared gate/mode byte this step switches on. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared status halfword this step publishes. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* Payload-local halfword table the gate byte indexes: 0x12, 0x13, 0x14, 0x15
 * and 0x16, in the last ten bytes of this overlay. */
extern u16 D_801F33B4[]; /* @source 0x801F33B4 @kind table */

/* @source 0x801F2C6C
 * @behavior Publishes the shared status halfword 0x801490A8 from the
 *           payload-local halfword table at 0x801F33B4 indexed by the signed
 *           shared gate byte 0x801490C7. The table holds five halfwords,
 *           0x12 through 0x16, so gate values zero through four publish 0x12
 *           to 0x16 and any other gate value indexes outside that run.
 *
 *           Constant and store evidence from the original bytes: the gate cell
 *           0x801490C7 is read once with `lui`/`lb` (signed byte, so the index
 *           sign-extends), the table base 0x801F33B4 is scaled by `sll` by one
 *           and added into $at for the `lhu`, and the status cell 0x801490A8
 *           takes the `sh`. The epilogue is a bare `jr $ra` with a delay `nop`,
 *           so the 0x2C-byte span is one complete leaf with a single return
 *           path and no argument registers live at entry. The payload holds
 *           this span's address as the second word of the code pointer run at
 *           0x801F3360, so the loader's handler table is its only caller and
 *           the parameter roles stay structural.
 *
 *           Shape: the sibling publishModeStatusFromTable3ad4World00Area00513_
 *           801F2C84 in target emi/world00/area005/13 carries the same
 *           two-statement body. A payload comparison of the two spans showed
 *           ten of eleven words equal, the single difference being that
 *           sibling's own table low immediate (0x3AD4 there, 0x33B4 here),
 *           which is a per-overlay relocation and not clone eligibility, so
 *           the statement order, operand forms and declaration style are
 *           transcribed while every literal here is written from this span's
 *           own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishModeStatusFromTable33b4World00Area02113_801F2C6C(void) {
  s32 mode;

  mode = D_801490C7;
  D_801490A8 = D_801F33B4[mode];
}
