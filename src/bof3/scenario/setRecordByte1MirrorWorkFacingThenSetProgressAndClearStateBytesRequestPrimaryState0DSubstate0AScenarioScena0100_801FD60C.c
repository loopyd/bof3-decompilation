#include "bof3/bof3.h"

/* This target's scratchpad work-object pointer cell, kept in its named symbol
 * form so every access re-materialises the cell address.
 * @source 0x1F800044 @kind unknown
 */
extern u8* scenarioScena0100Work;
/* Shared mode byte the work facing is derived from.
 * @source 0x80145E98 @kind data
 */
extern u8 D_80145E98;

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015BAC4(u8 resource_id);
void func_8015C088(void);

/* @source 0x801FD60C
 * @behavior Entry 6 of this overlay's 18-entry record callback table at
 * 0x801FE308, whose pointer word sits at 0x801FE320; the overlay dispatcher
 * dispatchRecordCallbackByByte7AScenarioScena0100_801FD444 (0x801FD444)
 * selects it with unsigned byte 0x7A of a record, handing that record in $a0
 * and the address of D_80144E90 in $a1, so the overlay reaches this handler
 * only indirectly, only the record argument is read and the handler returns
 * nothing. It runs the shared front-end startup helper func_8015C088
 * (0x8015C088), publishes state value 0x0A into the shared scenario progress
 * byte g_ScenarioProgress (0x80146864), clears the shared state bytes
 * 0x80146865 and 0x80146866, stores 4 in the record's byte at offset 1,
 * clears its halfword at 0x7E and its byte at 0x7B, and stores the same 0x0A
 * in its byte at 0x77. It then publishes the shared mode byte D_80145E98
 * (0x80145E98) with bit 2 toggled into the scratch work object's facing byte
 * at offset 8, passes that byte to the resident resource helper func_8015BAC4
 * (0x8015BAC4) and sets bit 3 of the work object's byte at offset 7, and
 * finally requests sub-state 0x0A in D_80146875 and primary scenario state
 * 0x0D in D_80146874. The one local carries the 0x0A this handler stores at
 * three sites, because the original materialises that constant once and keeps
 * it in $s1 across the resource-helper call. The record stores are
 * offset-anchored because this checkout localises only the work facing byte
 * (offset 8) and the shared mode byte; the halfword at 0x7E is the counter
 * other overlays increment, and the roles of bytes 1, 0x77 and 0x7B in this
 * record family are not established. The 0x20-byte frame keeps $s0 for the
 * record across the two calls. The two anchor values 0x0A and 0x0D are
 * request parameters, so the name keeps the address qualifier.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordByte1MirrorWorkFacingThenSetProgressAndClearStateBytesRequestPrimaryState0DSubstate0AScenarioScena0100_801FD60C(
    u8* record) {
  u8 state;
  func_8015C088();
  state = 0xA;
  g_ScenarioProgress = state;
  D_80146865 = 0;
  D_80146866 = 0;
  record[1] = 4;
  *(u16*)(record + 0x7E) = 0;
  record[0x7B] = 0;
  record[0x77] = state;
  scenarioScena0100Work[8] = D_80145E98 ^ 4;
  func_8015BAC4(scenarioScena0100Work[8]);
  scenarioScena0100Work[7] |= 8;
  D_80146875 = state;
  D_80146874 = 0xD;
}
