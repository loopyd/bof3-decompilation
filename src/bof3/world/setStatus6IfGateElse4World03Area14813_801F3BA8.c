#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3BA8
 * @behavior Loads the shared gate byte D_801490C7 and stores 6 into the shared
 * status halfword D_801490A8 while the gate byte is nonzero, and 4 when the
 * gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus6IfGateElse4World03Area14813_801F3BA8, D_801490C7, D_801490A8, 6, 4)
