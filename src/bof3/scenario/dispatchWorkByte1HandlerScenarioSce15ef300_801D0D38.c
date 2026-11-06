#include "bof3/emi/dispatchers.h"

/* This overlay's work-state handler run based at 0x801D27CC: nineteen in-image
 * code-pointer words, entry 0 = 0x801D0D7C (the countdown arming handler
 * armCountdownAndAdvanceWorkByte1ScenarioSce15ef300_801D0D7C, which advances the
 * very work byte this dispatcher reads) through entry 18 = 0x801D16BC at
 * 0x801D2814, the last word before the payload ends at 0x801D2818. It starts
 * immediately after the separate three-word run at 0x801D27C0
 * ({0x801D0C48, 0x801D0CC4, 0x801D0D18}), whose first words belong to another
 * handler family. The base word sits inside the Splat `asm` span labelled
 * func_801D27B0 and the following words inside the T_801D27D0 `textbin`, so no
 * reviewed data row places it; this dispatcher is its only reference.
 * @source 0x801D27CC @kind table
 */
extern void (*D_801D27CC[])(void);

/* @source 0x801D0D38
 * @behavior Overlay work-state dispatcher of this payload: it loads the
 * work-object pointer published at the scratchpad pointer cell 0x1F800044
 * before it creates its frame, takes that object's unsigned byte at offset
 * +0x1 (the work byte the handler table's entries advance), scales it by four
 * and calls that entry of the overlay's work-state handler run at 0x801D27CC
 * with no arguments, discarding any result. The table's entry 0 is 0x801D0D7C
 * and its entry 18 is 0x801D16BC, so work bytes 0 through 18 select the
 * overlay's step handlers. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra at 0x10 across the indirect call, and the
 * index load's delay slot, the table load's delay slot and the `jalr` delay
 * slot all stay nops. No `jal` and no word of this payload addresses
 * 0x801D0D38, so the overlay loader outside the payload enters it; nothing in
 * the payload binds it to a pointer run. Its 68 bytes are the 17-instruction
 * dispatcher shape that the exact siblings
 * dispatchWorkByte1HandlerScenarioScena0300_801F6E70 (0x801F6E70) and
 * dispatchWorkByte1HandlerScenarioScena0700_801F6DCC (0x801F6DCC) record, which
 * differ from it only in the table-address immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerScenarioSce15ef300_801D0D38, D_801D27CC);
