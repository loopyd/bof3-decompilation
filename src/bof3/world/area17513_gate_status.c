#include "bof3/bof3.h"
#include "world/gate_status.h"

extern s8 D_801490C7;
extern u16 D_801490A8;

/* @source 0x801F3970
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x63 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x62
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus63IfGateElse62World04Area17513_801F3970, D_801490C7, D_801490A8, 0x63, 0x62)

/* @source 0x801F3A34
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x70 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus70IfGateElseFFFFWorld04Area17513_801F3A34, D_801490C7, D_801490A8, 0x70, 0xFFFF)

/* @source 0x801F3A5C
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x70 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus70IfGateElseFFFFWorld04Area17513_801F3A5C, D_801490C7, D_801490A8, 0x70, 0xFFFF)

/* @source 0x801F3B4C
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x7E into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus7EIfGateElseFFFFWorld04Area17513_801F3B4C, D_801490C7, D_801490A8, 0x7E, 0xFFFF)

/* @source 0x801F3B74
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x7E into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus7EIfGateElseFFFFWorld04Area17513_801F3B74, D_801490C7, D_801490A8, 0x7E, 0xFFFF)

/* @source 0x801F3C4C
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x8A into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus8AIfGateElseFFFFWorld04Area17513_801F3C4C, D_801490C7, D_801490A8, 0x8A, 0xFFFF)

/* @source 0x801F3C74
 * @behavior Loads the shared gate byte D_801490C7 and stores 0x8A into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xFFFF
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatus8AIfGateElseFFFFWorld04Area17513_801F3C74, D_801490C7, D_801490A8, 0x8A, 0xFFFF)

/* @source 0x801F3CA4
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xA0 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0x9F
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatusA0IfGateElse9FWorld04Area17513_801F3CA4, D_801490C7, D_801490A8, 0xA0, 0x9F)

/* @source 0x801F3E28
 * @behavior Loads the shared gate byte D_801490C7 and stores 0xF8 into the
 * shared status halfword D_801490A8 while the gate byte is nonzero, and 0xF9
 * when the gate byte is zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DEFINE_GATE_STATUS(setStatusF8IfGateElseF9World04Area17513_801F3E28, D_801490C7, D_801490A8, 0xF8, 0xF9)
