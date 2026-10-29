#include "bof3/bof3.h"

extern u8 D_801490C7;
extern u16 D_801490A8;
extern u8 D_801F2944[2];

/* @source 0x801F3D94
 * @behavior Publishes the shared gate byte D_801490C7 into slot 0 of the
 * two-byte pair D_801F2944 and stores 0xFFFF into the shared status halfword
 * D_801490A8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishGateByteSlot0World04Area17513_801F3D94(void) {
  u8 gate;

  gate = D_801490C7;
  D_801490A8 = 0xFFFF;
  D_801F2944[0] = gate;
}
