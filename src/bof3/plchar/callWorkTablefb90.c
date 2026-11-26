#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801CFB84: sixteen code pointers, entry 0 =
 * 0x801CE448 through entry 15 = 0x801CF91C. The Splat `textbin` row codePointerTablefb84
 * places the words, so the run is only referenced here. This dispatcher
 * indexes the run from its word 3, so its base sits at 0x801CFB90 and the entries
 * it can reach are 0x801CE770 through 0x801CF91C, the shared main-executable
 * handler 0x801B2D54 included at two of them. */
extern void (*D_801CFB90[])(void); /* @source 0x801CFB90 @kind data */

/* @source 0x801CE72C
 * @behavior Overlay step dispatch on the work record's byte 2: loads the
 *           work-record pointer published at the scratchpad pointer cell
 *           0x1F800044 once, takes that record's unsigned byte at offset 2,
 *           scales it by four and calls the matching entry of this overlay's
 *           handler run at 0x801CFB90 with no arguments, discarding any result.
 *           The original does not bound the index before the scaled table load.
 *           It reads no other state and writes none; the pointer-cell load
 *           precedes frame creation, the 0x18-byte frame only keeps $ra at 0x10
 *           across the indirect call, and the slots after the index byte and
 *           after the table load stay nops. Its own address is the word at
 *           0x801CFBD4, entry 4 of the seven-entry code-pointer run at
 *           0x801CFBC4 that the sibling dispatcher at 0x801CFAFC indexes from
 *           0x801CFBC8, so that dispatcher is what enters it.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callWorkTablefb90, callWorkTablefb90, D_801CFB90);
