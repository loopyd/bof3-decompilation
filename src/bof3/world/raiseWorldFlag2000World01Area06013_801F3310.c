#include "bof3/bof3.h"

/* Shared world-flags halfword, bound at 0x80146258 by the shared map. Bit
 * 0x2000 raised here is the gate the world area handler at 0x801F55EC tests
 * before it enters mode 4 and clears work byte 2; the bit's own role is not
 * named by that consumer, so this source keeps it address-anchored as the
 * mask 0x2000. */
extern u16 D_80146258; /* @source 0x80146258 @kind unknown */

/* @source 0x801F3310
 * @behavior Raises bit 0x2000 of the shared world-flags halfword D_80146258
 *   with one halfword read-modify-write: the original loads the word through
 *   a folded lui/lhu base, leaves the load delay slot empty, ORs the mask into
 *   the loaded value and stores the low halfword back through a
 *   re-materialised lui $at base. Takes no arguments, returns nothing, keeps
 *   no frame and reads or writes no other state. The overlay has no jal to
 *   this address; its only in-image reference is the code-pointer word at
 *   0x801F3484 in the trailing table of the same overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorldFlag2000World01Area06013_801F3310(void) {
  D_80146258 |= 0x2000;
}
