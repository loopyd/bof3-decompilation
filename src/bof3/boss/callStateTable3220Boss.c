#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the twelve words from 0x800C31F0 through
 * 0x800C321C are in-image code pointers, this payload's own functions
 * interleaved with shared main-executable handlers (0x801E52E8, 0x801E35D8,
 * 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E5288), and word 0 is this payload's 0x800C1BB0. The run sits inside the
 * reviewed trailing `textbin` row D_800C3174, so it is only referenced here, and
 * the sibling byte-1 dispatcher at 0x800C1B6C indexes that same interior base.
 * This dispatcher is word 6 of that run, at 0x800C3208, and it indexes the
 * six-word table that follows the run at 0x800C3220: 0x801E3BAC, 0x801E3BD0,
 * 0x801E3BAC, 0x801E4490, this payload's 0x800C1CB0 and 0x801E4928. The table
 * also lies in the D_800C3174 row and ends where the next run starts, at
 * 0x800C3238, so the run stays one reviewed row and this declaration binds only
 * the word the scaled index is added to. */
extern void (*D_800C3220[])(void); /* @source 0x800C3220 @kind data */

/* @source 0x800C1C6C
 * @behavior Overlay state dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of the six-word handler table at 0x800C3220 with no arguments, so no
 * argument register is loaded and the callee's value in $v0 is what this
 * function returns. The original does not bound the index before the scaled
 * table load. It reads no other state and writes none; the pointer-cell load
 * precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the slots after the index byte and after the table
 * load stay nops. The only in-image reference to this span is the pointer word
 * at 0x800C3208, word 6 of the run based at 0x800C31F0, and the payload holds
 * no jal naming it, so the overlay stepper that walks the run reaches it as
 * entry 6 of the byte-1 dispatch. Same body as the exact sibling dispatchers at
 * 0x800C197C, 0x800C1EC8 and 0x800C2030 of this overlay, which read the same
 * record byte: their 0x44-byte spans are word-identical apart from the table
 * address in the scaled load word, which occupies the single `%lo` slot those
 * spans share, so those bodies supplied the statement shape and this span
 * supplied its own table literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable3220Boss, callStateTable3220Boss, D_800C3220);
