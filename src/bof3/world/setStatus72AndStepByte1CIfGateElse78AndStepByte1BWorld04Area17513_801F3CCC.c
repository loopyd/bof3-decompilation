#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801F294D;
extern u8 D_801F294E;

/* @source 0x801F3CCC
 * @behavior Loads the shared gate byte D_801490C7 and, while the gate byte is
 * nonzero, stores 0x72 into the shared status halfword D_801490A8 and 0x0C into
 * byte 1 of the overlay-wide step cell group at 0x801F294D; when the gate byte
 * is zero it stores 0x78 and 0x0B there instead. Either way it then clears step
 * byte 2 at 0x801F294E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus72AndStepByte1CIfGateElse78AndStepByte1BWorld04Area17513_801F3CCC(void) {
  s32 value;

  value = D_801490C7;
  if (value == 0) {
    D_801490A8 = 0x78;
    value = 0x0B;
  } else {
    D_801490A8 = 0x72;
    value = 0x0C;
  }
  D_801F294D = value;
  D_801F294E = 0;
}
