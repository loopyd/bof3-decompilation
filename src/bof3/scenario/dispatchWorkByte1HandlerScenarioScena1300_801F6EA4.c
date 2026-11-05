#include "bof3/bof3.h"

typedef void (*ScenarioScena1300WorkStateHandler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FBEF0, twelve words
 * (0x801FBEF0..0x801FBF1C): entry 0 is 0x801F6EE8, the byte-1 handler
 * clearWorkTintArmCountdownAndAdvanceWorkByte1ScenarioScena1300_801F6EE8, entry
 * 1 is 0x801F6F24, the handler that source advances the work dispatch byte to,
 * and entry 11 is 0x801F7C24. The word at 0x801FBF20 (0x00A70200) begins the
 * non-pointer data that follows the run. The address is the Splat `textbin`
 * boundary at payload offset 0x52F0, so the reviewed spelling is the raw map
 * name. */
extern ScenarioScena1300WorkStateHandler D_801FBEF0[];

/* @source 0x801F6EA4
 * @behavior This overlay's per-frame handler dispatcher for the scratchpad work
 * object's byte at offset 1: it loads the work-object pointer cell at
 * 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its
 * frame, takes that object's unsigned byte at offset 1 (`lbu $v0,0x1($v0)`),
 * scales it by four (`sll $v0,$v0,2`) and invokes that slot of the
 * overlay-local in-image code-pointer run based at 0x801FBEF0 (`lui $at,%hi` /
 * `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no
 * result consumed. The run is twelve words long (entry 0 = 0x801F6EE8 through
 * 0x801F7C24 at 0x801FBF1C) and the word at 0x801FBF20 is not a pointer. It
 * reads no state beyond that byte and writes none; its 0x18-byte frame exists
 * only to keep $ra across the indirect call, whose delay slot stays a nop, and
 * the pointer-cell load precedes frame creation. No word in this payload
 * addresses 0x801F6EA4, so nothing in the overlay invokes it directly. Its 68
 * bytes are instruction-for-instruction identical to the byte-1 dispatcher of
 * the exact sibling scena03/00 at 0x801F6E70
 * (dispatchWorkByte1HandlerScenarioScena0300_801F6E70), differing only in the
 * table-address immediate word, and the same stream is the one those exact
 * siblings record at scena01/00 0x801F73B8, scena07/00 0x801F6DCC, scena08/00
 * 0x801F6F48, scena15/00 0x801F6F38 and scena00 0x801F6DC0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1300_801F6EA4(void) {
  D_801FBEF0[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
