#include "bof3/emi/dispatchers.h"

typedef void (*Boss05016Handler)(void);

/*
 * Handler run based at 0x800C1B4C, the table this dispatcher selects from. It
 * starts at the seventh word of the pointer run the payload keeps from
 * 0x800C1B34 to its end at 0x800C1B88, reviewed as the textbin row at
 * 0x800C1B34, so this declaration binds only the word the scaled index is
 * added to and the run stays one row. Entry 0 is this overlay's entry point
 * 0x800C1854, entries 1 through 11 are the shared main-executable handlers
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288, and entries
 * 12 through 14 are this overlay's inert handlers 0x800C18F0, 0x800C18F8 and
 * 0x800C1900.
 */
extern Boss05016Handler D_800C1B4C[]; /* @source 0x800C1B4C @kind data */

/* @source 0x800C1810
 * @behavior Overlay step dispatch on work-record state byte 1: loads the
 * work-record pointer published at the scratchpad pointer cell 0x1F800044
 * once, takes that record's unsigned byte at offset 1, scales it by four and
 * calls the matching entry of this overlay's handler run based at 0x800C1B4C
 * with no arguments, discarding any result. The chosen entry is read from the
 * table itself rather than from a frame copy, and the original does not bound
 * the index before the scaled table load. It reads no other state and writes
 * none; the pointer-cell load precedes frame creation, the 0x18-byte frame
 * only keeps $ra at 0x10 across the indirect call, and the slots after the
 * index byte and after the table load stay nops. No word of this payload names
 * 0x800C1810, neither a jal immediate nor a pointer word, so the overlay loader
 * that steps this run is its caller and the function takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, callStateByte1Table7, D_800C1B4C);
