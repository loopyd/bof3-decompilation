#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_80146875;

/* @source 0x801F4F6C
 * @behavior Loads the signed shared gate byte D_801490C7, stores 0xFFFF into
 * the shared status halfword D_801490A8, and then stores 0x1E into the shared
 * sub-state byte D_80146875 while the gate byte is nonzero, and 0x0A when
 * the gate byte is zero; it takes no arguments and returns nothing. This
 * overlay's dispatch table at 0x801F6610 holds its address at 0x801F6664, so it
 * is one of the overlay's table-dispatched handlers. It has the same instruction
 * shape and the same three globals as the area175 family member
 * setStatusFFFFSubstate2IfGateElse3World04Area17513_801F386C
 * (emi/world04/area175/13), differing only in the two selected constants
 * (0x1E/0x0A here versus 2/3 there), so the two functions are not
 * byte-identical duplicates and this target owns its own address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFFFFSubstate1EIfGateElse0AWorld04Area17413_801F4F6C(void) {
  D_801490A8 = 0xFFFF;
  if (D_801490C7 != 0) {
    D_80146875 = 0x1E;
  } else {
    D_80146875 = 0x0A;
  }
}
