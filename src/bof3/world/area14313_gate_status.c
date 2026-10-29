#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3ACC
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x4A into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x49
 * when the gate byte is zero. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus4AIfGateElse49World03Area14313_801F3ACC, D_801490C7, D_801490A8, 0x4A, 0x49)
