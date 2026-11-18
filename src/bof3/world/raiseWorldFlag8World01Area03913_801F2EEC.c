#include "bof3/bof3.h"

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. Bit 0x8
 * raised here is cleared by scenario sources with an andi 0xFFF7 and tested
 * together with 0x8014625A by world and UI handlers; the bit's own role is not
 * named by those consumers, so this source keeps it address-anchored as the
 * mask 0x8. */
extern u16 D_80146258; /* @source 0x80146258 @kind unknown */

/* @source 0x801F2EEC
 * @behavior Raises bit 0x8 of the shared world-flags halfword D_80146258 with
 *   one halfword read-modify-write: the original loads the word through a
 *   folded lui/lhu base, leaves the load delay slot empty, ORs the mask into
 *   the loaded value and stores the low halfword back through a re-materialised
 *   lui $at base. Takes no arguments, returns nothing and keeps no frame. The
 *   payload has no jal to this address; its only in-image reference is the
 *   code-pointer word at 0x801F4BBC in the trailing pointer run of the same
 *   overlay, which starts at 0x801F4B90 with the payload's first function
 *   0x801F2C04.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorldFlag8World01Area03913_801F2EEC(void) {
  D_80146258 |= 0x8;
}
