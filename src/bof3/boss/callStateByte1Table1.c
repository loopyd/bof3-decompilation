#include "bof3/emi/dispatchers.h"

/* This overlay's step-handler run: twelve in-image code pointers starting at
 * 0x800C1DA0, inside the reviewed `textbin` row D_800C1D78, which spans
 * 0x800C1D78 through 0x800C1E18. Entry 0 is this payload's step-0 publisher
 * 0x800C1864 and entries 1 through 11 are the shared main-executable handlers
 * 0x801E52E8, 0x801E35D8, 0x801E3638, 0x801E3980, 0x801E3A00, 0x801E3B68,
 * 0x801E4AE8, 0x801E4D8C, 0x801E4F64, 0x801E52E8 and 0x801E5288. The
 * three-word run after it at 0x800C1DD0 belongs to this payload's index
 * dispatcher 0x800C18BC, so the base named here is this run's own first word.
 * Splat places that whole row, so it is only referenced here. */
extern void (*D_800C1DA0[])(void); /* @source 0x800C1DA0 @kind data */

/* @source 0x800C1820
 * @behavior Overlay step dispatch on work-record byte 1: loads the work-record
 * pointer published at the scratchpad pointer cell 0x1F800044, takes that
 * record's unsigned byte at offset 1, scales it by four and calls the matching
 * entry of this overlay's handler run based at 0x800C1DA0 with no arguments. The
 * original does not bound the index before the scaled table load, and the slots
 * after the index byte and after the table load stay nops. It reads no other
 * state and writes none; the pointer-cell load precedes frame creation, the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * selected handler's $v0 is left untouched by the epilogue, so this shim returns
 * whatever that handler returns. The payload holds neither a jal nor a pointer
 * word naming 0x800C1820, so the overlay loader that steps the handler run is
 * its caller and the function takes no arguments. Seeded from the exact sibling
 * callByte1Table1ddc of this target, whose
 * measurement is exact: the two spans are equal word for word except the single
 * %lo word of the scaled table load at 0x800C1844, 0x800C1DA0 here against
 * 0x800C1DDC there, both in the one 0x8000 page.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateByte1Table1, callStateByte1Table1, D_800C1DA0);
