#include "bof3/bof3.h"

typedef void (*ScenarioScena0100RecordCallback)(u8* record, u8* work);

extern ScenarioScena0100RecordCallback D_801FE308[];
extern u8 D_80144E90;

/* @source 0x801FD444
 * @behavior Dispatches one record callback selected by the record's unsigned byte field
 * at offset 0x7A through this overlay's callback-pointer table at 0x801FE308, passing
 * the record pointer and the address of the shared main-RAM byte D_80144E90; it reads
 * no other state and writes none. The scena01/00 twin of scena00's
 * dispatchRecordCallbackByByte7A (0x801FC7D0) and scena18's dispatchRecordCallback_scena18
 * (0x801F6CAC), which select the same record byte through their own tables but pass the
 * word held at 0x8014686C instead of a pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7AScenarioScena0100_801FD444(u8* record) {
  D_801FE308[record[0x7A]](record, &D_80144E90);
}
