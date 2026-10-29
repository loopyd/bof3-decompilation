#include "bof3/bof3.h"

extern u8 D_80144F28;

void func_8015B580(void* arg0, s32 arg1);

/* @source 0x801F5BD8
 * @behavior Overlay wrapper: sets flag 4 of the shared EXE flag block at
 *           0x80144F28 through the shared flag helper at 0x8015B580; takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedFlagBit4World02Area07714_801F5BD8(void) {
  func_8015B580(&D_80144F28, 4);
}
