#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the words from 0x800C2740 through 0x800C279C are
 * in-image code pointers, this payload's own functions interleaved with the
 * shared main-executable handlers at 0x801E.... The run sits inside the
 * 0x800C2720 `textbin` row that the reviewed Splat layout places, so it is only
 * referenced here. This dispatcher indexes it from the interior word 0x800C2788,
 * the nineteenth pointer word and the twenty-seventh word of the row, so the run
 * stays one reviewed row and this declaration binds only that word. From there
 * the six reachable entries are 0x800C1A04, 0x800C1A4C, 0x800C1B9C, 0x800C1BF4,
 * 0x800C1BFC and 0x800C1C04, the last word of the row. */
extern void (*D_800C2788[])(void); /* @source 0x800C2788 @kind data */

/* @source 0x800C19C0
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of the handler run based at the interior word 0x800C2788 with no
 * arguments. The original does not bound the index before the scaled table load.
 * It reads no other state and writes none; the pointer-cell load precedes frame
 * creation, the 0x18-byte frame only keeps $ra at 0x10 across the indirect call,
 * and the slots after the index byte and after the table load stay nops. The
 * epilogue leaves $v0 untouched, so this shim returns whatever the selected
 * handler returns. No jal in this payload names 0x800C19C0; the one word that
 * does name it is the run entry at 0x800C276C, the twelfth pointer word from
 * 0x800C2740, so the overlay loader that steps the pointer run is its caller.
 * The span was word-identical to the dispatcher at 0x800C18FC, which was lifted
 * exact, apart from the table address in its load word: 0x800C2788 here against
 * 0x800C2770 there, a 0x18 difference inside one 0x8000 page that occupies the
 * single `%lo` slot both spans share, so that body supplied the statement shape
 * and this span supplied its own table literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchStateByte2Table2788BossBoss00413_800C19C0, D_800C2788);
