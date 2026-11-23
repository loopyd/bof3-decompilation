#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena0500WorkByte1Handler)(void);

/* Overlay-local work-byte-1 handler table at 0x801FD5A8, indexed by the work
 * object's unsigned byte at offset 1: its six code-pointer words run from
 * incrementClearCounterScenario (0x801F743C, word 0)
 * through invokeHelperScenarioScena0500_801F7640 (0x801F7640, word 5), and the
 * first non-pointer word, 0x1C301C1C, sits immediately after them at 0x801FD5C0,
 * so the table ends there. Word 1 is
 * renderCursorTickProgress
 * (0x801F745C), word 2
 * invokeHelperScenarioScena0500_801F74CC (0x801F74CC), word 3
 * incrementSetCounterScenario (0x801F7530) and word 4
 * func_801F7554 (0x801F7554). Every word is an overlay handler address reached
 * only through this dispatcher; the two non-pointer words are followed by the
 * five-word code-pointer run 0x801FD5C8..0x801FD5D8 and the three-word
 * outer-state table at 0x801FD5DC.
 * @source 0x801FD5A8 @kind table
 */
extern ScenarioScena0500WorkByte1Handler D_801FD5A8[];

/* @source 0x801F73F8
 * @behavior Per-frame handler dispatcher of this overlay's first step: it reads
 * the work-object pointer cell at scratchpad 0x1F800044 before the frame is
 * created, takes that object's unsigned byte at offset 1, scales it by four and
 * calls that entry of the overlay-local handler table at 0x801FD5A8, so the work
 * byte selects which of the overlay's six work-byte-1 handlers runs this frame.
 * It reads no other state and writes none, takes no arguments and returns
 * nothing of its own, and its 0x18-byte frame only keeps $ra across the indirect
 * call while the handler's $v0 is discarded. No in-image jal and no image word
 * names this address, so the overlay loader reaches it as the overlay's entry
 * point and the work byte's range is the caller's to bound. Its 68 bytes were
 * identical to the exact sibling dispatchers
 * dispatchWorkByte1HandlerScenarioScena0300_801F844C (0x801F844C) and
 * dispatchWorkByte1HandlerScenarioScena1500_801F77F8 (0x801F77F8) in all
 * seventeen words except the table-load immediate at offset 0x24, which differs
 * because each overlay dispatches through its own table.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1,
                      callFirstStepHandler,
                      D_801FD5A8);
