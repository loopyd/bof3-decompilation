#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F30A4
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 9 when
 * the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(stateStatusNineUnless, D_801490C7, D_801490A8, 0xFFFF, 9)

/* @source 0x801F30CC
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 10 when
 * the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(stateStatusTenUnless, D_801490C7, D_801490A8, 0xFFFF, 10)

/* @source 0x801F30F4
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 11 when
 * the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(stateStatusElevenUnless, D_801490C7, D_801490A8, 0xFFFF, 11)

/* @source 0x801F3144
 * @behavior Loads the shared gate byte D_801490C7 and, while it equals 1,
 * stores 0xD into the shared status halfword D_801490A8; when the gate byte
 * differs from 1 it stores 0xFFFF into D_801490A8 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStatusThirteenGate(void) {
  if (D_801490C7 == 1) {
    D_801490A8 = 0xD;
  } else {
    D_801490A8 = 0xFFFF;
  }
}

/* @source 0x801F316C
 * @behavior Loads the shared gate byte D_801490C7 and, when it equals 1,
 * stores 0xE into the shared status halfword D_801490A8; when the gate byte
 * differs from 1 it stores 0xFFFF into D_801490A8 instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusFourteenIfGateElseFFFFWorld01Area05613_801F316C(void) {
  if (D_801490C7 == 1) {
    D_801490A8 = 0xE;
  } else {
    D_801490A8 = 0xFFFF;
  }
}
