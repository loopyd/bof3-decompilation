#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE8D0[];

/* @source 0x801F7928
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer through the scratchpad pointer slot at
 * 0x1F800044, takes its unsigned state byte at offset 1, scales it by four and
 * invokes that entry of the overlay-local handler-pointer table D_801FE8D0 with
 * no arguments. The table holds two in-image code pointers, entry 0 = 0x801F796C
 * and entry 1 = 0x801F7AD0, and it is a continuation of the 51-entry handler run
 * that starts at 0x801FE894 (D_801FE8D0 is that run's entry 15). It reads no
 * other state and writes none: the pointer slot is loaded before the 0x18-byte
 * frame is created, and that frame only keeps $ra across the indirect call while
 * the callee's return value in $v0 is discarded. The bytes are the exact twin of
 * the same overlay's first-entry dispatcher at 0x801F6F48
 * (dispatchWorkByte1HandlerScenarioScena0800_801F6F48) and of the continuation
 * dispatcher at 0x801F707C, which read the same work byte through earlier
 * entries of that run. No code or pointer word inside this overlay addresses
 * 0x801F7928; the only in-image reference to it lies outside the overlay, in the
 * shared handler-pointer table of the concurrently loaded etc/game overlay
 * (source word 0x801C7DB8 inside its data blob D_801C7C70), which is the word
 * after 0x801F707C and two words after 0x801F6F48.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0800_801F7928(void) {
  D_801FE8D0[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
