#include "bof3/bof3.h"

/* Runtime-filled record pointer cell at 0x801F22BC, the last word of this
 * overlay's trailing data block. func_801EEC04 walks the 0x80 step records at
 * 0x801F12BC, publishes the record it is about to step into this cell with
 * `sw` and restores nothing, so every consumer sees the current record.
 * func_801F0960, func_801F09E0, func_801F0B34, func_801F0C7C, func_801F0DA4
 * and func_801F11AC reload the cell once per access, and this dispatcher reads
 * it once. The word is zero on disc. */
extern u8 *D_801F22BC; /* @source 0x801F22BC @kind data */

/* Handler run inside the trailing data block. Above this base the words at
 * 0x801F125C through 0x801F1294 are fifteen in-image code pointers
 * (func_801EEF14 first), 0x801F1298 through 0x801F12A7 hold the byte ramp
 * 0c/0e/0a/0c/08/0a/06/08/04/06/03/04/01/02/01/01, and from 0x801F12A8 the run
 * resumes with func_801F0960, func_801F09E0, func_801F0B34, func_801F0C7C and
 * func_801F0DA4 followed by zeros. This dispatcher indexes the run from
 * 0x801F12A8, so its reachable entries are those five handlers. The whole span
 * sits inside the reviewed 0x801F125C `textbin` row, which the target layout
 * places, so the run is only referenced here. */
extern void (*D_801F12A8[])(void); /* @source 0x801F12A8 @kind data */

/* @source 0x801F091C
 * @behavior Overlay step dispatch on the current record's byte 1: loads the
 * record pointer published at 0x801F22BC once, takes that record's unsigned
 * byte at offset 1, scales it by four and calls the matching entry of this
 * overlay's handler run based at 0x801F12A8 with no arguments, discarding any
 * result. The chosen entry is read from the table itself, not from a frame
 * copy, and the original does not bound the index before the scaled table load,
 * so index 5 and above reach the zero words that follow the five handlers.
 * It reads no other state and writes none; the pointer-cell load precedes
 * frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 * indirect call, and the slot after the index byte and the slot after the
 * table load stay nops. The record stepper func_801EEC04 calls it, and it
 * takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte1Table12a8BmagicMagic12603_801F091C(void) {
  D_801F12A8[D_801F22BC[1]]();
}
