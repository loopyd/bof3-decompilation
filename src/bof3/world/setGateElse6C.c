#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7; /* @source 0x801490C7 @kind unknown */
extern u16 D_801490A8; /* @source 0x801490A8 @kind unknown */

/* @source 0x801F2D54
 * @behavior Entry 3 of the target handler table at 0x801F37AC. Loads the shared
 * gate byte D_801490C7 into a local and stores 0xFFFF into the shared status
 * halfword D_801490A8 while the gate byte is nonzero, and 0x6C when the gate
 * byte is zero. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the 0x28-byte span is byte-identical to
 * emi/world04/area191/13 0x801F2FEC, which carries source through the shared
 * WORLD_DEFINE_GATE_STATUS macro of include/world/gate_status.h; one byte
 * comparison of the two payload spans at their own offsets found no differing
 * word, because both read the same absolute cells outside either payload. The
 * macro owns the branch and both arm constants, so the invocation below
 * carries only this function's own cells and constants.
 */
WORLD_DEFINE_GATE_STATUS(setGateElse6C, D_801490C7, D_801490A8, 0xFFFF, 0x6C)
