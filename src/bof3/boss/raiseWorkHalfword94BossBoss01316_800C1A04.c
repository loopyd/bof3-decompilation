#include "bof3/bof3.h"

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* @source 0x800C1A04
 * @behavior Entry 1 of the handler run the byte-indexed dispatcher
 * dispatchIndexedHandlerBossBoss01316_800C18FC selects from, its base the run
 * interior word 0x800C2650 of this payload's D_800C25D8 data row: entry 0 is
 * 0x800C1934, this body is entry 1, entry 2 is the inert handler 0x800C1A2C and
 * entry 3 is 0x800C1A78. Only the one word at 0x800C2654 holds this address and
 * no jal in the payload names it, so the overlay dispatcher is what reaches it.
 * It raises the unsigned halfword +0x94 of the current enemy work record
 * published at 0x801EB4E8 from zero to 1, and leaves that halfword untouched
 * when it is already nonzero, so the store is reached once per zero value. The
 * body is a leaf with no frame: it takes no arguments, returns nothing and
 * keeps no state across the single store. Roles of the halfword are not
 * established here; the main program's own record writers own them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseWorkHalfword94BossBoss01316_800C1A04(void) {
  if (FIELD_REF(u16, D_801EB4E8, 0x94u) == 0) {
    FIELD_REF(u16, D_801EB4E8, 0x94u) = 1;
  }
}
