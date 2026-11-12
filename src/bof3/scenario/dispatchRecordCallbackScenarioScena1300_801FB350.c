#include "bof3/bof3.h"

typedef void (*ScenarioScena1300RecordCallback)(u8* record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local per-record callback table at 0x801FBFB8: thirteen code-pointer
 * words running from incrementWorkHalfword7EScenarioScena1300_801FB390
 * (0x801FB390, word 0) and requestPrimaryState3Substate5ScenarioScena1300_801FB3A4
 * (0x801FB3A4, word 1) through noopHandlerScenarioScena1300_801FBEE8
 * (0x801FBEE8, word 12), indexed by byte 0x7A of a record. It begins
 * immediately after the data word 0x020AFFFF at 0x801FBFB4 and ends immediately
 * before the zero word at 0x801FBFEC; every entry is reached only through this
 * dispatcher, and the two entries this overlay already lifts for another reason
 * document their own slot.
 * @source 0x801FBFB8 @kind table
 */
extern ScenarioScena1300RecordCallback D_801FBFB8[];

/* @source 0x801FB350
 * @behavior Dispatches this overlay's per-record callback selected by the
 * record's unsigned byte field at offset 0x7A through the overlay-local
 * callback-pointer table at 0x801FBFB8 (thirteen entries: word 0
 * incrementWorkHalfword7EScenarioScena1300_801FB390, word 1
 * requestPrimaryState3Substate5ScenarioScena1300_801FB3A4, word 2
 * requestPrimaryState6Substate19ScenarioScena1300_801FB3DC, word 3
 * setScenarioFlagBit1CThenRequestStateEntry2CProgress0Substate11ScenarioScena1300_801FB414,
 * word 4
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1300_801FBB78,
 * word 5
 * copyPartyListIntoMirrorThenInvokePartyHelperArmEffectGroupAndResetEffectScenarioScena1300_801FBBC4,
 * word 6
 * armEffectGroupThenClearWorkRecordsAndResetEffectScenarioScena1300_801FBC88,
 * word 7
 * armEffectGroupFromPartyMirrorThenResetWorkCountAndPublishPartyListScenarioScena1300_801FBCC4,
 * word 8 the still-anchored func_801FBD84 and word 9
 * invokePartyHelperForWorkRecordPartyBytesThenArmEffectGroupAndResetEffectScenarioScena1300_801FBDD8,
 * word 10
 * invokeHelperArgThenHelperScenarioScena1300_801FBE98,
 * word 11 invokeHelperArgThenHelperScenarioScena1300_801FBEC0 and word 12
 * noopHandlerScenarioScena1300_801FBEE8), passing the record pointer unchanged
 * as the first argument and the word held in the shared main-RAM scenario flag
 * cell D_8014686C as the second. It reads no other state and writes none: the
 * 0x18-byte frame exists only to keep $ra across the indirect call and the
 * callback's return value in $v0 is discarded. The 64 payload bytes are
 * instruction-for-instruction identical to the exact record-callback
 * dispatchers dispatchRecordCallbackScenarioScena0800_801FE018 (0x801FE018) and
 * dispatchRecordCallbackScenarioScena0300_801FC46C (0x801FC46C), which differ
 * only in the table-entry load immediate and keep the same slot contract. No
 * in-image jal targets this address: its only image word is 0x801FBF38, word 1
 * of the four-word pointer run 0x801FBF34 through 0x801FBF40 that the zero word
 * at 0x801FBF44 terminates, so the overlay reaches it only indirectly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena1300_801FB350(u8* record) {
  D_801FBFB8[record[0x7A]](record, D_8014686C);
}
