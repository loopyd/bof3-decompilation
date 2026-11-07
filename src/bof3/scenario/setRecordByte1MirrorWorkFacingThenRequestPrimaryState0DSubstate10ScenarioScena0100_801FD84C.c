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
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;
/* @source 0x80146875 @kind data */
extern u8 D_80146875;

void func_8015BAC4(u8 resource_id);
void func_8015C088(void);

/* @source 0x801FD84C
 * @behavior Entry 9 of this overlay's 18-entry record callback table at
 * 0x801FE308, whose pointer word sits at 0x801FE32C; the overlay dispatcher
 * dispatchRecordCallbackByByte7AScenarioScena0100_801FD444 (0x801FD444)
 * selects it with unsigned byte 0x7A of a record, handing that record in $a0
 * and the address of D_80144E90 in $a1, so the overlay reaches this handler
 * only indirectly and only the record argument is read. It runs the shared
 * front-end startup helper func_8015C088 (0x8015C088), stores 4 in the
 * record's byte at offset 1, clears its halfword at 0x7E and its byte at 0x7B,
 * stores 0x0B in its byte at 0x77, then publishes the shared mode byte
 * D_80145E98 (0x80145E98) with bit 2 toggled into the scratch work object's
 * facing byte at offset 8, passes that byte to the resident resource helper
 * func_8015BAC4 (0x8015BAC4), sets bit 3 of the work object's byte at offset
 * 7, stores sub-state 0x10 to D_80146875, clears the shared state byte
 * D_80146867 and requests primary scenario state 0x0D in D_80146874. The
 * scratch pointer cell at 0x1F800044 is read once per
 * access, so the source keeps it as the named symbol instead of one local
 * pointer: the original materialises all three loads. The record stores are
 * offset-anchored because this checkout localises only the work facing byte
 * (offset 8) and the shared mode byte; the halfword at 0x7E is the counter
 * other overlays increment, and the roles of bytes 1, 0x77 and 0x7B in this
 * record family are not established. Takes the record pointer and returns
 * nothing; the 0x18-byte frame keeps $s0 for the record across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordByte1MirrorWorkFacingThenRequestPrimaryState0DSubstate10ScenarioScena0100_801FD84C(u8* record) {
  func_8015C088();
  record[1] = 4;
  *(u16*)(record + 0x7E) = 0;
  record[0x7B] = 0;
  record[0x77] = 0xB;
  scenarioScena0100Work[8] = D_80145E98 ^ 4;
  func_8015BAC4(scenarioScena0100Work[8]);
  scenarioScena0100Work[7] |= 8;
  D_80146875 = 0x10;
  D_80146867 = 0;
  D_80146874 = 0xD;
}
