#include "bof3/bof3.h"

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. Its
 * observed low bits carry the front-end record gate (bits 0-2), the mode flags
 * 0x40/0x80/0x100/0x180/0x240 and 0x400; the roles of bits 4 and 12 cleared here
 * are unknown, so this source keeps them address-anchored as the mask 0x1010. */
extern u16 D_80146258; /* @source 0x80146258 @kind unknown */

/* @source 0x801F4A9C
 * @behavior Clears bits 0x0010 and 0x1000 of the shared world-flags halfword
 * D_80146258 with one halfword read-modify-write: the original loads the word
 * through a folded lui/lhu base, leaves the load delay slot empty, ANDs the
 * complement mask 0xEFEF into the loaded value and stores the low halfword back
 * through a re-materialised lui $at base. Takes no arguments, returns nothing,
 * keeps no frame and reads or writes no other state. It is the inverse of the
 * raising routine at 0x801F4A7C, which ORs exactly these two bits in the same
 * word, so the body is that sibling's with the operator and immediate inverted.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorldFlags1010World04Area17013_801F4A9C(void) {
  D_80146258 &= 0xEFEF;
}
