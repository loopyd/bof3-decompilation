#include "bof3/bof3.h"

typedef void (*ScenarioScena0800RecordCallback)(u8* record, u32 arg1);

extern ScenarioScena0800RecordCallback D_801FE9C0[];
extern u32 D_8014686C;

/* @source 0x801FE018
 * @behavior Dispatches this overlay's per-record callback selected by the
 * record's unsigned byte field at offset 0x7A through the overlay-local
 * callback-pointer table at 0x801FE9C0 (eleven entries: func_801FE058,
 * func_801FE0A0, func_801FE0E0, func_801FE698, func_801FE6DC,
 * invokeHelperArgScenarioScena0800_801FE740,
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena0800_801FE760,
 * func_801FE7AC,
 * func_801FE800, func_801FE840 and invokeHelperArgScenarioScena0800_801FE874),
 * passing the record pointer unchanged and the word held in the shared main-RAM
 * cell D_8014686C as the second argument. It reads no other state and writes
 * none: the 0x18-byte frame exists only to keep $ra across the indirect call
 * and the callback's return value in $v0 is discarded. This address is itself
 * entry 48 (the word reading 0x801FE018) of the 51-entry handler run at
 * 0x801FE894 that the work-object state dispatcher
 * dispatchWorkByte1HandlerScenarioScena0800_801F6F48 (0x801F6F48) indexes with
 * the scratchpad work byte at 0x1F800045 (splat additionally splits that run at
 * 0x801FE938), and the SCENA08 twin of the exact scena15 dispatcher at
 * 0x801FDD80 (dispatchRecordCallbackScenarioScena1500_801FDD80), the exact
 * scena18 dispatcher at 0x801F6CAC (dispatchRecordCallback_scena18) and the
 * exact scena00 dispatcher at 0x801FC7D0 (dispatchRecordCallbackByByte7A).
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0800_801FE018(u8* record) {
  D_801FE9C0[record[0x7A]](record, D_8014686C);
}
