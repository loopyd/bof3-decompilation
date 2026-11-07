#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena0400WorkByte1Handler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FA180: nine handler
 * words, 0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C, 0x801F6FF8,
 * 0x801F7064, 0x801F77EC, 0x801F78F4 and 0x801F7A64. This dispatcher indexes it
 * from 0x801FA198, so its base is entry 6 of that run and the entries the run
 * still holds from there are 0x801F77EC, 0x801F78F4 and 0x801F7A64, after which
 * the word at 0x801FA1A4 is 0x00010000 and begins the non-pointer data that
 * follows the run.
 * @source 0x801FA198 @kind table
 */
extern ScenarioScena0400WorkByte1Handler D_801FA198[];

/* @source 0x801F77A8
 * @behavior Overlay step dispatch on work-record byte 1 from the seventh word
 * of this overlay's handler run: it loads the work-object pointer from the
 * scratchpad cell 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before
 * it creates its frame, takes that object's unsigned byte at offset 1
 * (`lbu $v0,0x1($v0)`), scales it by four (`sll $v0,$v0,2`) and invokes that
 * slot of the overlay-local code-pointer run based at 0x801FA198
 * (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no
 * arguments and no result consumed. 0x801FA198 is entry 6 of the run based at
 * 0x801FA180, so the index is relative to 0x801F77EC, 0x801F78F4 and
 * 0x801F7A64; the index is not bounded before the scaled table load. It reads
 * no state beyond the pointer cell and that byte, writes none, and its
 * 0x18-byte frame exists only to keep $ra at 0x10 across the indirect call,
 * whose delay slot stays a nop. Neither an in-image `jal` nor any in-image word
 * holds 0x801F77A8, so the frame call that reaches this dispatcher comes from
 * outside the payload. Its 68 bytes are instruction-for-instruction identical
 * to the step dispatchers of other EMI scenario overlays and to this target's
 * byte-1 dispatcher at 0x801F6D00; they differ only in the table immediate
 * word, which is a discriminating immediate of this overlay, so each target and
 * each table base owns its own source.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioScena0400_801F77A8, D_801FA198);
