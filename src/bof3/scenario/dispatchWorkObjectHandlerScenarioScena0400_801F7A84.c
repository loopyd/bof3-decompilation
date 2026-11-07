#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena0400WorkObjectHandler)(void);

/* Overlay-local run of work-object handler code pointers based at 0x801FA1E4:
 * eight words, 0x801F7AC8, 0x801F7B00, 0x801F7BC0, 0x801F8124, 0x801F9D48,
 * 0x801F9DB4, 0x801F9DAC and 0x801FA010, after which the word at 0x801FA204 is
 * 0x801F8160 and begins the separate pointer run this overlay's progress
 * dispatcher indexes. This dispatcher indexes the run from its own base.
 * @source 0x801FA1E4 @kind table
 */
extern ScenarioScena0400WorkObjectHandler D_801FA1E4[];

/* @source 0x801F7A84
 * @behavior Work-object dispatch on the work byte the handlers advance: it
 * loads the work-object pointer from the scratchpad cell 0x1F800044
 * (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame,
 * takes that object's unsigned byte at offset 1 (`lbu $v0,0x1($v0)`), scales
 * it by four (`sll $v0,$v0,2`) and invokes that slot of the overlay-local
 * code-pointer run based at 0x801FA1E4 (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed.
 * That run holds the eight work-object handlers entry 0 of which
 * clearWorkTableByte25AndAdvanceWorkByte1ScenarioScena0400_801F7AC8 documents,
 * so a handler that advances the same byte moves the next frame to its
 * neighbour; the index is not bounded before the scaled table load. It reads
 * no state beyond the pointer cell and that byte and writes none, and its
 * 0x18-byte frame exists only to keep $ra at 0x10 across the indirect call,
 * whose delay slot stays a nop. Neither an in-image `jal` nor any in-image
 * word holds 0x801F7A84, so the frame call that reaches this dispatcher comes
 * from outside the payload. Its 68 bytes are instruction-for-instruction
 * identical to this overlay's other two byte-1 dispatchers, at 0x801F6D00
 * (run base 0x801FA180) and 0x801F77A8 (word 6 of that same run), differing
 * only in the table-base immediate pair, which is how each copy names the
 * pointer run it owns.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkObjectHandlerScenarioScena0400_801F7A84, D_801FA1E4);
