#include "bof3/bof3.h"

typedef void (*Scena1500FrameHandler)(void);

extern u8* D_1F800044;
extern Scena1500FrameHandler func_801FE57C[];

/* @source 0x801F6F38
 * @behavior Dispatches the overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer cell at 0x1F800044, takes its unsigned
 * state byte at offset 1, scales it by four and invokes that entry of the
 * overlay-local 27-entry handler table at 0x801FE57C (entries 0..4 are
 * 0x801F6F7C, 0x801F6FCC, 0x801F7078, 0x801F7100 and 0x801F717C, the last entry
 * is 0x801F9000); it reads no other state and writes none, and the 0x18-byte
 * frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1500_801F6F38(void) {
  func_801FE57C[D_1F800044[1]]();
}
