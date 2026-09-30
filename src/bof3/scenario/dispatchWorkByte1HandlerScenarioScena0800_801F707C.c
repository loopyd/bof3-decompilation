#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE8A0[];

/* @source 0x801F707C
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer through the scratchpad pointer slot at
 * 0x1F800044, takes its unsigned state byte at offset 1, scales it by four and
 * invokes that entry of the overlay-local handler-pointer table D_801FE8A0 with
 * no arguments. The table holds twelve in-image code pointers, entry 0 =
 * 0x801F70C0 through entry 11 = 0x801F774C, and it is the continuation of the
 * 51-entry handler run that starts at 0x801FE894 (D_801FE8A0 is that run's entry
 * 3). It reads no other state and writes none: the pointer slot is loaded before
 * the 0x18-byte frame is created, and that frame only keeps $ra across the
 * indirect call while the callee's return value in $v0 is discarded. The bytes
 * are the exact twin of the same overlay's first-entry dispatcher at 0x801F6F48
 * (dispatchWorkByte1HandlerScenarioScena0800_801F6F48), which reaches run entry
 * 0 through the same work byte, and of the scena15/00 continuation dispatcher at
 * 0x801F77F8. No code or pointer word inside this overlay addresses 0x801F707C
 * or 0x801F7928; the only in-image references to either lie outside it, in the
 * shared handler-pointer table of the concurrently loaded etc/game overlay
 * (source word 0x801C7DB4 inside its data blob D_801C7C70), where 0x801F707C and
 * 0x801F7928 are the two consecutive words after 0x801F6F48.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0800_801F707C(void) {
  D_801FE8A0[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
