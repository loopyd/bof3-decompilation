#include "bof3/bof3.h"

typedef void (*ScenarioScena0800WorkStateHandler)(void);

extern ScenarioScena0800WorkStateHandler D_801FE8D8[];

/* @source 0x801F796C
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad work
 * object: reads the work-object pointer through the scratchpad pointer slot at
 * 0x1F800044, takes its unsigned byte at offset 2, scales it by four and invokes
 * that entry of the overlay-local handler-pointer table D_801FE8D8 with no
 * arguments. D_801FE8D8 is entry 17 of the 51-entry per-frame handler-pointer run
 * at 0x801FE894 (entries 0 through 50, ending at 0x801FE960) and its two entries
 * are 0x801F79B0 (clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0800_801F79B0)
 * and 0x801F79EC. It reads no other state and writes none: the pointer slot is
 * loaded before the 0x18-byte frame is created, and that frame only keeps $ra
 * across the indirect call while the callee's return value in $v0 is discarded.
 * The bytes are the same shape as the same overlay's work-byte-1 dispatchers at
 * 0x801F6F48, 0x801F707C and 0x801F7928, which reach earlier entries of the same
 * run; this one differs by taking the work-object byte at offset 2. Its only
 * in-image reference is the word 0x801F796C in the run itself, at the two-entry
 * sub-table 0x801FE8D0 (run entry 15) that 0x801F7928 indexes when work byte 1
 * reads zero; no code section of this overlay calls it directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0800_801F796C(void) {
  D_801FE8D8[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
