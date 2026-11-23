#include "bof3/bof3.h"
#include "world/gate_status.h"

/* Shared gate byte this step publishes and switches on. */
extern s8 D_801490C7; /* @source 0x801490C7 @kind data */

/* Shared status halfword this step publishes. */
extern u16 D_801490A8; /* @source 0x801490A8 @kind data */

/* @source 0x801F2CD8
 * @behavior Loads the shared gate byte D_801490C7 and, while that byte is
 *           nonzero, stores 1 into the shared status halfword D_801490A8; when
 *           the gate byte is zero it stores 0x14 into D_801490A8 instead. Takes
 *           no arguments and returns nothing; leaf with no frame, and the
 *           published value is materialised in $v0 and fills the branch delay
 *           slot.
 *           Evidence: the 0x28 bytes 0x801F2CD8..0x801F2CFF are one complete
 *           leaf with a single `jr $ra` and no frame or call. The gate read is
 *           a signed `lb`, and the status store is one `sh` after the branch
 *           merge with the value in $v0, which is the value form of this
 *           statement order rather than a store inside each arm. The body is
 *           the shared gate-status shape; the exact sibling
 *           setStatus14IfGateElse13World02Area11213_801F3870
 *           (emi/world02/area112/13@0x801F3870) carries the same sequence
 *           with its own literals 0x14 and 0x13, and no byte-identical span
 *           exists in any payload of this tree.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(stateStatus1GateElse14,
                         D_801490C7, D_801490A8, 1, 0x14)
