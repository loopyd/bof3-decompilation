#include "bof3/bof3.h"

typedef void (*ScenarioScena0300RecordCallback)(u8* record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local per-record callback table at 0x801FD00C: nine code-pointer
 * words running from func_801FC4AC (0x801FC4AC, word 0) through
 * func_801FC6C4 (0x801FC6C4, word 8), indexed by byte 0x7A of a record. It
 * begins immediately after the two data words 0x000100FF/0x000001FF at
 * 0x801FD004/0x801FD008 and ends immediately before the data word 0x014F2363
 * at 0x801FD030; every entry is reached only through this dispatcher.
 * @source 0x801FD00C @kind table
 */
extern ScenarioScena0300RecordCallback D_801FD00C[];

/* @source 0x801FC46C
 * @behavior Dispatches this overlay's per-record callback selected by the
 * record's unsigned byte field at offset 0x7A through the overlay-local
 * callback-pointer table at 0x801FD00C (nine entries: func_801FC4AC,
 * setScenarioFlagBit10ThenInvokeHelperScenarioScena0300_801FC4F4,
 * setScenarioFlagBit11ThenInvokeHelperScenarioScena0300_801FC520,
 * setScenarioFlagBit12ThenInvokeHelperScenarioScena0300_801FC54C,
 * setScenarioFlagBit13ThenInvokeHelperScenarioScena0300_801FC578,
 * func_801FC5FC, noopHandlerScenarioScena0300_801FC688, func_801FC690 and
 * func_801FC6C4), passing the record pointer unchanged as the first argument
 * and the word held in the shared main-RAM cell D_8014686C as the second
 * (slot 7, func_801FC690, is documented as receiving exactly that pair). It
 * reads no other state and writes none: the 0x18-byte frame exists only to
 * keep $ra across the indirect call and the callback's return value in $v0 is
 * discarded. The address is itself word 6 (the word reading 0x801FC46C) of the
 * ten-word pointer run at 0x801FCFAC that no in-image jal targets, and its 64
 * bytes are instruction-for-instruction identical to the exact record-callback
 * dispatchers dispatchRecordCallbackScenarioScena0800_801FE018 (0x801FE018),
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 (0x801FA5B8) and
 * dispatchRecordCallbackScenarioScena1500_801FDD80 (0x801FDD80), differing only
 * in the table-entry load immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0300_801FC46C(u8* record) {
  D_801FD00C[record[0x7A]](record, D_8014686C);
}
