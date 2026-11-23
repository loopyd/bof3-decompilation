#include "bof3/emi/dispatchers.h"

/* The handler run this body indexes is the six-word code-pointer run at
 * 0x801CF62C: entry 0 = 0x801CE404, entry 1 = 0x801CF328, entry 2 = 0x801CF544,
 * entry 3 = 0x801CE610, entry 4 = 0x801CF534 and entry 5 = 0x801CF554. This
 * target's Splat row at 0x801CF62C still declares that run as the function
 * func_801CF62C although every one of its six words is a code pointer, so the
 * run is only referenced here, under the row's own name and with no second map
 * row. This dispatcher indexes it from that first word, so its base is the run's
 * own base and it reaches entry 0 through entry 5. The sixteen-word pointer run
 * at 0x801CF5EC, which the byte-slot dispatchers of this overlay index, ends
 * four words below this run and is a separate table. */
extern void (*func_801CF62C[])(void); /* @source 0x801CF62C @kind data */

/* Seeded from the lift of the same body at 0x801CF5A8 in this payload, which
 * measured exact before this seed was written. These 68 bytes agree with that
 * span word for word except the `%lo` word of the scaled table load, where each
 * body names its own run base: 0x801CF62C here, the interior entry 3 at
 * 0x801CF638 there, both on the same `%hi` page, so the statement order, the
 * macro spelling and the declaration style were carried over with this
 * function's own table symbol and name.
 */
/* @source 0x801CF564
 * @behavior Overlay step dispatch on the work record halfword at offset 0x2C:
 *           loads the work-record pointer published at the scratchpad pointer
 *           cell 0x1F800044 once, takes that record's unsigned halfword at offset
 *           0x2C, scales it by four and calls the matching entry of this
 *           overlay's handler run based at 0x801CF62C with no arguments,
 *           discarding any result. The original does not bound the index before
 *           the scaled table load, so a halfword past entry 5 reads beyond the
 *           run. It reads no other state and writes none; the pointer-cell load
 *           precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 *           across the indirect call, and the slots after the index halfword and
 *           after the table load stay nops. Neither this function's address nor
 *           the table base appears in any word of this payload and no jal
 *           targets it, so it is an overlay entry reached from outside the
 *           payload; its prologue and return path alone place the boundary,
 *           which ends at 0x801CF5A8 where the interior-entry dispatcher of the
 *           same run begins.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkTablef62(void) {
  func_801CF62C[SPAD_PTR_SLOT(u16, 0x44u)[0x16]]();
}
