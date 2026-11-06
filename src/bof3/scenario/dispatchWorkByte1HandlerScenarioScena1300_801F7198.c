#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena1300WorkStateHandler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FBEF0, twelve words
 * (0x801FBEF0..0x801FBF1C). The address 0x801FBEFC is word 3 of that run: from
 * there the words are 0x801F71DC, 0x801F7248, 0x801F72A0, 0x801F735C,
 * 0x801F7444, 0x801F763C, 0x801F7958, 0x801F7A1C and 0x801F7C24. The word at
 * 0x801FBF20 (0x00A70200) begins the non-pointer data that follows the run, and
 * the run's base is the Splat `textbin` boundary at payload offset 0x52F0. */
extern ScenarioScena1300WorkStateHandler D_801FBEFC[];

/* @source 0x801F7198
 * @behavior Per-frame step dispatcher of this overlay: it loads the work-record
 * pointer from the scratchpad cell 0x1F800044 (`lui $v0,0x1F80` then
 * `lw $v0,0x44($v0)`) before it creates its frame, takes that record's unsigned
 * byte at offset 1 (`lbu $v0,0x1($v0)`), scales it by four (`sll $v0,$v0,2`) and
 * invokes that slot of the overlay-local code-pointer run at 0x801FBEFC
 * (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no
 * arguments and no result consumed, so byte 1 selects 0x801F7248, word 4 of the
 * twelve-word run based at 0x801FBEF0. It reads no state beyond that byte and
 * writes none; its 0x18-byte frame exists only to keep $ra across the indirect
 * call, whose delay slot stays a nop. No word in this payload addresses
 * 0x801F7198, so nothing in the overlay references it directly. Its 68 bytes are
 * instruction-for-instruction identical to the exact byte-1 dispatcher of this
 * overlay at 0x801F6EA4 (dispatchWorkByte1HandlerScenarioScena1300_801F6EA4),
 * which indexes the run's base at 0x801FBEF0; the two differ only in the table
 * immediate word, and the same stream is the one the exact sibling scena15/00
 * dispatchers record at 0x801F77F8, 0x801F7AA8 and 0x801F8488.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioScena1300_801F7198, D_801FBEFC);
