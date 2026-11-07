#include "bof3/emi/dispatchers.h"

/* This overlay's work-state handler run based at 0x801D2800: six in-image
 * code-pointer words holding 0x801D1420, 0x801D1460, 0x801D14DC, 0x801D15CC,
 * 0x801D1614 and 0x801D16BC, the last word at 0x801D2814 being the final word
 * before the payload ends at 0x801D2818. It is the tail of the longer run based
 * at 0x801D27CC (nineteen entries, 0x801D0D7C through 0x801D16BC): this base is
 * that run's entry 13. Entry 0 here is 0x801D1420, the countdown-clearing
 * handler clearWorkCountdownSetWorkByte6AndAdvanceWorkByte1ScenarioSce15ef300_801D1420
 * lifted exactly in this target, which advances the very work byte this
 * dispatcher reads, so work bytes 0 through 5 select this base's handlers. The
 * base word sits 0x30 bytes into the T_801D27D0 `textbin` span (0x801D27D0
 * through 0x801D280F) and the run's final two words fall inside the Splat `asm`
 * span labelled func_801D2810, whose eight bytes are the pointer words 0x801D1614
 * and 0x801D16BC rather than instructions, so no reviewed data row places the
 * base; this dispatcher is its only reference.
 * @source 0x801D2800 @kind table
 */
extern void (*D_801D2800[])(void);

/* @source 0x801D13DC
 * @behavior Overlay work-state dispatcher of this payload: it loads the
 * work-object pointer published at the scratchpad pointer cell 0x1F800044
 * before it creates its frame, takes that object's unsigned byte at offset +0x1
 * (the work byte the overlay's step handlers advance), scales it by four and
 * calls that entry of the handler run at 0x801D2800 with no arguments,
 * discarding any result. That run's entry 0 is 0x801D1420 and its entry 5 is
 * 0x801D16BC at 0x801D2814, so work bytes 0 through 5 select this dispatcher's
 * handlers; the index is not bounded before the scaled table load. It reads no
 * other state and writes none; the 0x18-byte frame only keeps $ra at 0x10 across
 * the indirect call, and the index load's delay slot, the table load's delay slot
 * and the `jalr` delay slot all stay nops. No `jal` and no word of this payload
 * addresses 0x801D13DC, so the overlay loader outside the payload enters it. Its
 * 68 bytes are the 17-instruction dispatcher shape the three exact siblings
 * dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38, ..._801D0EAC and
 * ..._801D1068 record; a span comparison against each leaves the same two
 * differing bytes at offset 0x24, the table address's low immediate (0x2800 here
 * against 0x27CC, 0x27D8 and 0x27EC there).
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef300_801D13DC, D_801D2800);
