#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler func_801FE894[];

/* @source 0x801F6F48
 * @behavior Dispatches the overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer cell at 0x1F800044, takes its unsigned
 * state byte at offset 1, scales it by four and invokes that entry of the
 * overlay-local handler table at 0x801FE894 (a 51-entry run of in-image code
 * pointers, entry 0 = func_801F6F8C, ending before the zero word at 0x801FE960)
 * with no arguments. It reads no other state and writes none; the pointer cell
 * is loaded before the 0x18-byte frame is created and that frame only keeps $ra
 * across the indirect call. The address is the first code boundary of this
 * overlay's main segment (segment offset 0x348) and the SCENA08 twin of the
 * exact scena15 dispatcher at 0x801F6F38
 * (dispatchWorkByte1HandlerScenarioScena1500_801F6F38), of the exact scena00
 * dispatcher at 0x801F6DC0 (dispatchWorkByte1Handler) and of the exact scena00
 * follower at 0x801F70F0 (dispatchWorkByte1Handler_801F70F0).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0800_801F6F48(void) {
  func_801FE894[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
