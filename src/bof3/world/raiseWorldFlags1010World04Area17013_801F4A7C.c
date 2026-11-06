#include "bof3/bof3.h"

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. Its
 * observed low bits carry the front-end record gate (bits 0-2), the mode flags
 * 0x40/0x80/0x100/0x180/0x240 and 0x400; the roles of bits 4 and 12 raised here
 * are unknown, so this source keeps them address-anchored as the mask 0x1010. */
extern u16 D_80146258; /* @source 0x80146258 @kind unknown */

/* @source 0x801F4A7C
 * @behavior Raises bits 0x0010 and 0x1000 of the shared world-flags halfword
 * D_80146258 with one halfword read-modify-write: the original loads the word
 * through a folded lui/lhu base, leaves the load delay slot empty, ORs the mask
 * into the loaded value and stores the low halfword back through a
 * re-materialised lui $at base. Takes no arguments, returns nothing, keeps no
 * frame and reads or writes no other state. The same-word store at 0x801F4A9C
 * clears exactly these two bits (andi 0xEFEF), and the overlay's own state
 * driver at 0x801F4DBC carries the identical raise inline.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorldFlags1010World04Area17013_801F4A7C(void) {
  D_80146258 |= 0x1010;
}
