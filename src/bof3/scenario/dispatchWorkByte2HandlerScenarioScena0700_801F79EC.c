#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local three-entry run of in-image code pointers based at 0x801FDDB4,
 * selected by the work object's byte at offset 0x02: 0x801F7A30, 0x801F7AB0 and
 * clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F7B68 (the exact sibling
 * whose own metadata records itself as entry 2 of this work-state handler table
 * and names the dispatcher below as the function indexing it). The separate
 * label D_801FDDC0 closes the run. The address sits inside the overlay's final
 * Splat asm block (the code-shaped boundary func_801FDD14), which defines the
 * label; the target map does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDDB4[];

/* @source 0x801F79EC
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 0x02: it loads the work-object pointer cell at 0x1F800044 (`lui
 * $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame, takes that
 * object's unsigned byte at offset 0x02 (`lbu $v0,0x2($v0)`), scales it by four
 * (`sll $v0,$v0,2`) and invokes that slot of the overlay-local in-image
 * code-pointer run based at 0x801FDDB4 (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed, so
 * that byte is the state index of the handler window this dispatcher reads. It
 * reads no state beyond that byte and writes none; its 0x18-byte frame exists
 * only to keep $ra across the indirect call, whose delay slot stays a nop, and
 * the pointer-cell load precedes frame creation. That run is three words long
 * (0x801F7A30, 0x801F7AB0 and the exact
 * clearWorkFlagsWhenNoCallbackRanScenarioScena0700_801F7B68) and is closed by
 * the separate label D_801FDDC0 at 0x801FDDC0. 0x801F79EC is
 * entry 6 (the word at 0x801FDD2C) of the ten-word work-object byte-1 handler
 * run based at 0x801FDD14 and that word is the only image reference to the
 * address, so the byte-1 dispatcher
 * dispatchWorkByte1HandlerScenarioScena0700_801F6DCC selects it when the work
 * object's byte 1 is 6. The 68 bytes are instruction-for-instruction identical
 * to the byte-2 dispatchers of the already-exact siblings
 * dispatchWorkByte2HandlerScenarioScena0700_801F6E10 (table 0x801FDD3C),
 * dispatchWorkByte2HandlerScenarioScena0700_801F700C (table 0x801FDD50),
 * dispatchWorkByte2HandlerScenarioScena0700_801F73C4 (table 0x801FDD70),
 * dispatchWorkByte2HandlerScenarioScena0700_801F75F4 (table 0x801FDD8C),
 * dispatchWorkByte2HandlerScenarioScena0700_801F786C (table 0x801FDDAC) and
 * this lane's dispatchWorkByte2HandlerScenarioScena0700_801F7B9C (table
 * 0x801FDDC0), differing only in instruction 9, the table-address immediate
 * word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0700_801F79EC(void) {
  D_801FDDB4[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
