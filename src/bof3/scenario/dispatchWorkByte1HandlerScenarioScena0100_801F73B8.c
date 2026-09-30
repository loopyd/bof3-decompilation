#include "bof3/bof3.h"

typedef void (*ScenarioScena0100WorkStateHandler)(void);

extern ScenarioScena0100WorkStateHandler func_801FE26C[];

/* @source 0x801F73B8
 * @behavior Dispatches this overlay's per-frame state handler from the scratchpad
 * work object: reads the work-object pointer cell at 0x1F800044, takes its
 * unsigned state byte at offset 1, scales it by four and invokes that entry of
 * the overlay-local handler table at 0x801FE26C with no arguments — five
 * in-image code pointers, entry 0 = 0x801F73FC and entry 4 = 0x801F7644, with
 * the non-code words at 0x801FE280 following the table. It reads no other state
 * and writes none; the 0x18-byte frame only keeps $ra across the indirect call,
 * and the pointer-cell load precedes frame creation. The bytes are the exact
 * twin of the scena08/00 dispatcher at 0x801F6F48
 * (dispatchWorkByte1HandlerScenarioScena0800_801F6F48), of the scena15/00
 * dispatchers at 0x801F6F38 and 0x801F77F8, and of the scena00 dispatcher at
 * 0x801F6DC0; each reads its own overlay's table through work byte 1. No code
 * pointer in this target's payload addresses 0x801F73B8, so its caller is
 * outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0100_801F73B8(void) {
  func_801FE26C[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
