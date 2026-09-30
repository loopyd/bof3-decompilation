#include "bof3/bof3.h"

/* @source 0x801F55F4
 * @behavior Overlay step handler: loads the scratch work record cursor from
 *           scratchpad pointer-slot 0x1F800044 and writes the word 5 to the
 *           record's +0x70 field; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkWord5World02Area08213_801F55F4(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u32*)(work + 0x70) = 5;
}
