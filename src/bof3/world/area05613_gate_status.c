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
WORLD_DEFINE_GATE_STATUS(setStatusNineUnlessGateWorld01Area05613_801F30A4, D_801490C7, D_801490A8, 0xFFFF, 9)

/* @source 0x801F30CC
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 10 when
 * the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatusTenUnlessGateWorld01Area05613_801F30CC, D_801490C7, D_801490A8, 0xFFFF, 10)

/* @source 0x801F30F4
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 11 when
 * the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatusElevenUnlessGateWorld01Area05613_801F30F4, D_801490C7, D_801490A8, 0xFFFF, 11)
