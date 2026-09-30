#include "bof3/bof3.h"

typedef void (*Scena1500FrameHandler)(void);

extern u8* D_1F800044;
extern Scena1500FrameHandler D_801FE5B0[];

/* @source 0x801F8488
 * @behavior Dispatches the overlay's per-frame handler from the scratchpad work
 * object: the work-object pointer is read through the scratchpad pointer cell at
 * 0x1F800044 before the frame is created, its unsigned state byte at offset 1 is
 * scaled by four and that entry of the overlay-local handler-pointer table
 * D_801FE5B0 is invoked with no arguments. D_801FE5B0 is a continuation of the
 * 27-entry handler run at 0x801FE57C (it is that run's entry 13) and holds 14
 * in-image code pointers, entry 0 = 0x801F84CC through entry 13 = 0x801F9000. It
 * reads no other state and writes none; the 0x18-byte frame only keeps $ra
 * across the indirect call and the callee's $v0 is discarded. The instruction
 * shape is the same as the dispatchers at 0x801F6F38 (run entry 0),
 * 0x801F77F8 (run entry 5) and 0x801F7AA8 (run entry 9) of this same overlay. No
 * word inside this overlay stores its address; the in-image reference is the
 * shared handler-pointer word 0x801C7EEC inside the data blob D_801C7C70 of the
 * concurrently loaded etc/game overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1500_801F8488(void) {
  D_801FE5B0[D_1F800044[1]]();
}
