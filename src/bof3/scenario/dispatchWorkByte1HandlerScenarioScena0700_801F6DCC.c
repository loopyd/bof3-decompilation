#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkStateHandler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FDD14, ten words
 * (0x801FDD14..0x801FDD38) ending at the next data label 0x801FDD3C: entry 0 is
 * 0x801F6E10, the byte-2 dispatcher dispatchWorkByte2HandlerScenarioScena0700_801F6E10,
 * and the run is the one this source selects with the work object's byte at
 * offset 1. The address is a code-shaped Splat boundary, so the reviewed
 * spelling is the raw map name. */
extern ScenarioScena0700WorkStateHandler func_801FDD14[];

/* @source 0x801F6DCC
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 1: loads the work-object pointer cell at 0x1F800044 (`lui $v0,0x1F80`
 * then `lw $v0,0x44($v0)`) before it creates its frame, takes that object's
 * unsigned byte at offset 1 (`lbu $v0,0x1($v0)`), scales it by four (`sll
 * $v0,$v0,2`) and invokes that slot of the overlay-local in-image code-pointer
 * run based at 0x801FDD14 (`lui $at,%hi` / `addu $at,$at,$v0` / `lw
 * $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed. The run
 * is ten words long (entry 0 = 0x801F6E10, the byte-2 dispatcher
 * dispatchWorkByte2HandlerScenarioScena0700_801F6E10, through 0x801F7E24 at
 * 0x801FDD38) and the next data label 0x801FDD3C follows it. It reads no state
 * beyond that byte and writes none; its 0x18-byte frame exists only to keep $ra
 * across the indirect call, whose delay slot stays a nop, and the pointer-cell
 * load precedes frame creation. No pointer word anywhere in this payload
 * addresses 0x801F6DCC, so nothing in the overlay invokes it directly. The 68
 * bytes are instruction-for-instruction identical to the byte-1 dispatchers of
 * the already-exact siblings scena01/00 at 0x801F73B8, scena08/00 at 0x801F6F48
 * (dispatchWorkByte1HandlerScenarioScena0800_801F6F48), scena15/00 at 0x801F6F38
 * and 0x801F77F8 and scena00 at 0x801F6DC0, differing only in the table-address
 * immediate word (instruction 9).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0700_801F6DCC(void) {
  func_801FDD14[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
