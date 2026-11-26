#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CF900: twenty-one code pointers, the words
 * 0x801CF900 through 0x801CF950, holding this payload's own functions plus the
 * shared main-executable handler 0x801B2D54 at entries 2, 5 and 11. The leading
 * sixteen words sit inside the 0x801CF900 `textbin` row and the trailing five in
 * the 0x801CF940 row, which the reviewed Splat layout places, so the run is only
 * referenced here. This dispatcher indexes the run from its first word, so its
 * base is entry 0 and the entries it can reach are entry 0 = 0x801CE448 through
 * entry 20 = 0x801CEE50. */
extern void (*workByte2HandlerTable[])(void); /* @source 0x801CF900 @kind data */

/* @source 0x801CE404
 * @behavior Overlay step dispatch on work-record byte 2: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044 once, takes that
 * record's unsigned byte at offset 2, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x801CF900 with no arguments,
 * discarding any result. The original does not bound the index before the
 * scaled table load, and that table load reads the word at the run's base, so
 * entry 0 is 0x801CE448. It reads no other state and writes none; the
 * pointer-cell load precedes frame creation, the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call, and the slot after the index byte and the
 * slot after the table load stay nops. Its only in-image reference is the word
 * 0x801CE404 at 0x801CF93C, entry 15 of the twenty-one-entry code-pointer run
 * at 0x801CF900, so the overlay loader that steps that run is what enters it:
 * it has no jal caller and takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, dispatchByte2Tablef900, workByte2HandlerTable);
