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
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015BAC4(u8 resource_id);
void func_8015C088(void);

/* @source 0x801FD6CC
 * @behavior Entry 7 of this overlay's 18-entry record callback table at
 * 0x801FE308, whose pointer word sits at 0x801FE324; the overlay dispatcher
 * callRecordCallbackScenario (0x801FD444)
 * selects it with unsigned byte 0x7A of a record, handing that record in $a0
 * and the address of D_80144E90 in $a1, so the overlay reaches this handler
 * only indirectly and only the record argument is read. It runs the shared
 * front-end startup helper func_8015C088 (0x8015C088), stores 4 in the
 * record's byte at offset 1, clears its byte at 0x7B and stores 0x0A in its
 * byte at 0x77, then publishes the shared mode byte D_80145E98 (0x80145E98)
 * with bit 2 toggled into the scratch work object's facing byte at offset 8,
 * passes that byte to the resident resource helper func_8015BAC4 (0x8015BAC4)
 * and sets bit 3 of the work object's byte at offset 7. It then requests
 * primary scenario state 0x0D in D_80146874 and sub-state 0x0D in D_80146875,
 * clearing the shared progress byte g_ScenarioProgress (0x80146864) and the
 * shared state bytes 0x80146865, 0x80146866 and 0x80146867 first. The scratch
 * pointer cell at 0x1F800044 is read once per access, so the source keeps it
 * as the named symbol instead of one local pointer: the original materialises
 * all three loads. The record stores are offset-anchored because this
 * checkout localises only the work facing byte (offset 8) and the shared mode
 * byte; the roles of bytes 1, 0x77 and 0x7B in this record family are not
 * established. Takes the record pointer and returns nothing; the 0x18-byte
 * frame keeps $s0 for the record across the two calls. Its 0xBC bytes are the
 * shape of this target's own
 * stateMirrorFacingPrimary
 * (0x801FD84C, table entry 9), which also clears the record halfword at 0x7E
 * and stores 0x0B, 0x10 and a single shared state byte, so those sites are
 * written from this span's own bytes; the shared state-byte clear run comes
 * from this target's own
 * setSubstate28ClearProgressAndSharedStateBytesAndRequestPrimaryState9ScenarioScena0100_801FD948
 * (0x801FD948). The two anchor values 0x0D and 0x0D are request parameters, so
 * the name keeps the address qualifier.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordByte1MirrorWorkFacingThenClearProgressAndStateBytesRequestPrimaryState0DSubstate0DScenarioScena0100_801FD6CC(
    u8* record) {
  func_8015C088();
  record[1] = 4;
  record[0x7B] = 0;
  record[0x77] = 0xA;
  scenarioScena0100Work[8] = D_80145E98 ^ 4;
  func_8015BAC4(scenarioScena0100Work[8]);
  scenarioScena0100Work[7] |= 8;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0xD;
  D_80146874 = 0xD;
}
