#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3870
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x14 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x13
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus14IfGateElse13World02Area11213_801F3870, D_801490C7, D_801490A8, 0x14, 0x13)
