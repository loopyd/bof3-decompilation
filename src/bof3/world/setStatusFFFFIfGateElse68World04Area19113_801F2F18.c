#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F2F18
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xFFFF into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x68
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatusFFFFIfGateElse68World04Area19113_801F2F18, D_801490C7, D_801490A8, 0xFFFF, 0x68)
