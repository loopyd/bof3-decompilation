#include "bof3/bof3.h"

extern s32 D_80149308;

/* @source 0x801F3FDC
 * @behavior Loads the shared X channel D_80149308 and, while it is at most
 * 0x2E0000, stores it into the X field at offset 0x34 of the work record
 * reached through the pointer slot at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySharedXToWorkXIfInRangeWorld04Area17513_801F3FDC(void) {
  s32 value;
  u8* work;

  value = D_80149308;
  if (value <= 0x2E0000) {
    work = SPAD_PTR_SLOT(u8, 0x44);
    *(s32*)(work + 0x34) = value;
  }
}
