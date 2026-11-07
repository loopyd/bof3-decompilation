#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena0400WorkByte1Handler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FA180: nine handler
 * words, 0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C, 0x801F6FF8,
 * 0x801F7064, 0x801F77EC, 0x801F78F4 and 0x801F7A64, so the table covers work
 * byte 1 values 0..8. The word at 0x801FA1A4 is 0x00010000 and begins the
 * non-pointer data that follows the run.
 * @source 0x801FA180 @kind table
 */
extern ScenarioScena0400WorkByte1Handler T_801FA180[];

/* @source 0x801F6D00
 * @behavior Per-frame step dispatcher of this overlay: it loads the work-object
 * pointer from the scratchpad cell 0x1F800044 (`lui $v0,0x1F80` then
 * `lw $v0,0x44($v0)`) before it creates its frame, takes that object's unsigned
 * byte at offset 1 (`lbu $v0,0x1($v0)`), scales it by four (`sll $v0,$v0,2`) and
 * invokes that slot of the overlay-local code-pointer run at 0x801FA180
 * (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no
 * arguments and no result consumed. The nine words of that run are the handlers
 * 0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C, 0x801F6FF8, 0x801F7064,
 * 0x801F77EC, 0x801F78F4 and 0x801F7A64, and the word that follows them at
 * 0x801FA1A4 is 0x00010000 rather than a code pointer, so byte 1 is this
 * overlay's step counter over that nine-entry chain: entry 0 (0x801F6D44) writes
 * 8 to that work object's byte 9 and advances byte 1, so the next frame
 * dispatches entry 1. It reads no state beyond the pointer cell and that byte
 * and writes none; its 0x18-byte frame exists only to keep $ra across the
 * indirect call, whose delay slot stays a nop. Neither an in-image `jal` nor any
 * in-image word holds 0x801F6D00, so the frame call that reaches this dispatcher
 * comes from outside the payload. Its 68 bytes are instruction-for-instruction
 * identical to the exact byte-1 dispatchers of other EMI scenario overlays, such
 * as dispatchWorkByte1HandlerScenarioScena1300_801F7198 (0x801F7198), which
 * index their own run's base; they differ only in the table immediate word, and
 * the run is a discriminating immediate of this target, so each target owns its
 * own address, boundary and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioScena0400_801F6D00, T_801FA180);
