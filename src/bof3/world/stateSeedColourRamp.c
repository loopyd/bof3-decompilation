#include "bof3/bof3.h"

extern u8* D_1F800044; /* @source 0x1F800044 @kind bss */
extern u8 D_801F59E8;
extern u8 D_801F59EC;

/* @source 0x801F2E64
 * @behavior Entry 0 of this overlay's second byte-1 dispatch table (0x801F5130,
 *           indexed by the dispatcher at 0x801F2E20). Reads the work record
 *           published at the scratchpad pointer cell 0x1F800044, stores zero
 *           into the record byte at offset 0x09 and then 1 into the record
 *           state byte at offset 0x01, so the next dispatch of that table
 *           takes its entry 1 (0x801F2EA0). It then seeds the two overlay
 *           cells that entry-1 handler works from: 0xF into the colour byte at
 *           0x801F59EC and zero into the phase byte at 0x801F59E8, the index
 *           entry 1 keeps in the eight-byte ramp table 0x801F511C and advances
 *           modulo 8, loading the indexed ramp byte into 0x801F59EC and
 *           stamping that colour into the RGB bytes of the semi-transparent G4
 *           it emits. Takes no arguments and returns nothing; the body is a
 *           leaf with no frame, the record pointer is loaded a second time for
 *           the second record access because the first store goes through it,
 *           and both overlay cell stores go through $at.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateSeedColourRamp(void) {
  D_1F800044[9] = 0;
  D_1F800044[1] = 1;
  D_801F59EC = 0xF;
  D_801F59E8 = 0;
}
