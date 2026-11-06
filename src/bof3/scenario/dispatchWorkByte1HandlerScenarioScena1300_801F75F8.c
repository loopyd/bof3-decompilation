#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena1300WorkStateHandler)(void);

/* Word 8 of the overlay-local in-image code-pointer run based at 0x801FBEF0:
 * twelve words from 0x801FBEF0 through 0x801FBF1C, and 0x801FBF10 holds
 * 0x801F763C. The word at 0x801FBF20 (0x00A70200) begins the non-pointer data
 * that follows the run. The address is inside the target's `textbin` boundary at
 * payload offset 0x52F0, which the two exact dispatchers of this overlay index at
 * their own bases (0x801FBEF0 and 0x801FBEFC). */
extern ScenarioScena1300WorkStateHandler D_801FBF10[]; /* @source 0x801FBF10 @kind data */

/* @source 0x801F75F8
 * @behavior This overlay's handler dispatcher for the scratchpad work object's
 * byte at offset 1: it loads the work-object pointer cell at 0x1F800044
 * (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame, takes
 * that object's unsigned byte at offset 1 (`lbu $v0,0x1($v0)`), scales it by four
 * (`sll $v0,$v0,2`) and invokes that slot of the code-pointer run's word 8 at
 * 0x801FBF10 (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr
 * $v0`) with no arguments and no result consumed, so byte 1 selects 0x801F763C.
 * It reads no state beyond that byte and writes none; its 0x18-byte frame exists
 * only to keep $ra across the indirect call, whose delay slot stays a nop, and the
 * pointer-cell load precedes frame creation. No word of this payload addresses
 * 0x801F75F8, so nothing in the overlay invokes it directly; the word appears
 * once in the etc/game overlay payload (offset 0x32688). Its 68 bytes are
 * instruction-for-instruction identical to the byte-1 dispatchers of this overlay
 * at 0x801F6EA4 (dispatchWorkByte1HandlerScenarioScena1300_801F6EA4) and
 * 0x801F7198 (dispatchWorkByte1HandlerScenarioScena1300_801F7198), which differ
 * only in the table-address immediate word.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioScena1300_801F75F8, D_801FBF10);
