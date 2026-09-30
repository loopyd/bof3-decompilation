#include "bof3/bof3.h"

typedef void (*Scena1500FrameHandler)(void);

extern u8* D_1F800044;
extern Scena1500FrameHandler D_801FE590[];

/* @source 0x801F77F8
 * @behavior Dispatches the overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer cell at 0x1F800044, takes its unsigned
 * state byte at offset 1, scales it by four and invokes that entry of the
 * overlay-local handler table continuation at 0x801FE590 (entries 0..3 are
 * 0x801F783C, 0x801F7900, 0x801F7920 and 0x801F79BC); it reads no other state
 * and writes none, and the 0x18-byte frame only keeps $ra across the indirect
 * call. The dispatcher at 0x801F6F38 reads the same work byte through the first
 * five entries of this table at 0x801FE57C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1500_801F77F8(void) {
  D_801FE590[D_1F800044[1]]();
}
