#include "bof3/bof3.h"

extern s8 D_801448EB;
extern s8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F3E54
 * @behavior Runs the shared front-end startup helper at 0x8015C088, then
 *           requests state entry 0x24 through the shared state byte at
 *           0x801448EB and stores the caller's progress byte into the shared
 *           byte at 0x801448EC. The argument is copied to $s0 before the call
 *           because it is only consumed by the store after it; the 0x18-byte
 *           frame keeps $ra and that copy across the call and each store
 *           materialises the 0x8014 page in $at on its own. The only in-image
 *           caller is 0x801F3CD4, which passes 0x0A or 0x0E after matching the
 *           packed argument against the 0x248000 and 0xA8000 pairs, so the
 *           parameter is a single progress/state value, not a pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStateEntry24AndProgressByteWorld02Area10613_801F3E54(s8 progress)
{
  func_8015C088();
  D_801448EB = 0x24;
  D_801448EC = progress;
}
