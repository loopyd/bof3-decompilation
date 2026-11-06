#include "bof3/bof3.h"

extern void func_8016663C(s32 arg0, s32 arg1);
extern s8 D_801490C7;  /* @source 0x801490C7 @kind data */
extern u32 D_80144F50; /* @source 0x80144F50 @kind data */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* @source 0x801F2C10
 * @behavior Loads the shared gate byte 0x801490C7 with a signed byte read
 *   (`lb`, so the cell is s8). While the gate byte is nonzero it selects 0x18;
 *   when the gate byte is zero it reads the main-RAM word 0x80144F50 with an
 *   unsigned bound of 0x14 (`lw`, `sltiu`, so the cell is u32 at that test),
 *   selecting 0x17 below the bound and otherwise calling func_8016663C(0x14, 0)
 *   and selecting 0x11. Every path joins on one store of the selected constant
 *   into the shared status halfword 0x801490A8 (`sh`); the frame is 0x18 bytes
 *   with only $ra saved. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus18IfGateElse17Or11ByWord80144F50World00Area02113_801F2C10(void) {
  s32 value;

  if (D_801490C7 == 0) {
    if (D_80144F50 < 0x14u) {
      value = 0x17;
    } else {
      func_8016663C(0x14, 0);
      value = 0x11;
    }
  } else {
    value = 0x18;
  }
  D_801490A8 = value;
}
