#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local three-entry run of in-image code pointers based at 0x801FDD3C,
 * selected by the work object's byte at offset 2: entry 0 is 0x801F6E54
 * (clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0700_801F6E54, the
 * exact sibling whose metadata records this run as that function's table),
 * entry 1 is 0x801F6E90 and entry 2 is
 * clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F6F14. The address sits
 * inside the overlay's final Splat asm block (the code-shaped boundary
 * func_801FDD14), which defines the label; the target map does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDD3C[];

/* @source 0x801F6E10
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 2: loads the work-object pointer cell at 0x1F800044 (`lui $v0,0x1F80`
 * then `lw $v0,0x44($v0)`) before it creates its frame, takes that object's
 * unsigned byte at offset 2 (`lbu $v0,0x2($v0)`), scales it by four (`sll
 * $v0,$v0,2`) and invokes that slot of the overlay-local in-image code-pointer
 * run based at 0x801FDD3C (`lui $at,%hi` / `addu $at,$at,$v0` / `lw
 * $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed. That run
 * is the three-entry handler table 0x801FDD3C whose entry 0 is 0x801F6E54 (the
 * exact sibling clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0700_801F6E54,
 * whose metadata records it as that table's entry 0), entry 1 is 0x801F6E90 and
 * entry 2 is the exact
 * clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F6F14, with the next data
 * label D_801FDD48 at 0x801FDD48 closing the run. It reads no state beyond that byte
 * and writes none; its 0x18-byte frame exists only to keep $ra across the
 * indirect call, whose delay slot stays a nop, and the pointer-cell load
 * precedes frame creation. The only pointer word addressing 0x801F6E10 in this
 * payload is the run's own entry at 0x801FDD14, so the byte-1 dispatcher
 * dispatchWorkByte1HandlerScenarioScena0700_801F6DCC is the caller the overlay
 * records for it. The 68 bytes are instruction-for-instruction identical to the
 * byte-2 dispatcher of the already-exact sibling scena12/00 at 0x801F71B8
 * (dispatchWorkByte2HandlerScenarioScena1200_801F71B8) and to the byte-1
 * dispatcher above except for the byte offset (instruction 4, 0x1 vs 0x2) and
 * the table-address immediate word (instruction 9).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0700_801F6E10(void) {
  D_801FDD3C[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
