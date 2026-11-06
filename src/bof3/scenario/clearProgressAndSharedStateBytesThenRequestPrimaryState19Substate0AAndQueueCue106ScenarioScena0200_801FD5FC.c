#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);
void func_8015DF18(u16 arg0);

/* @source 0x801FD5FC
 * @behavior Overlay record callback, entry 23 of the local callback table
 * D_801FE304 (pointer word at 0x801FE360), invoked as (record, work) by
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084 (0x801FD084), which
 * indexes that table with a record's unsigned byte field at offset 0x7A; it
 * reads neither argument. It runs the shared front-end startup helper
 * func_8015C088 (0x8015C088), requests secondary sub-state 0xA in the shared
 * sub-state byte D_80146875 (0x80146875), clears the shared progress byte
 * g_ScenarioProgress (0x80146864) and the shared bytes 0x80146865, 0x80146866
 * and 0x80146867, then requests primary scene state 0x19 through D_80146874
 * (0x80146874), which selects word 0x19 of the primary-state table D_801FE294 -
 * the pointer 0x801FB6C4 at 0x801FE2F8 - in
 * dispatchPrimaryStateHandlerScenarioScena0200_801F7A60 (0x801F7A60), and
 * finally queues the front-end cue 0x106 through func_8015DF18(u16)
 * (0x8015DF18). Takes no arguments and returns nothing; the 0x18-byte frame
 * only keeps $ra across the calls, every byte store materialises the 0x8014
 * page in $at on its own, the sub-state constant reaches $v0 straight after
 * func_8015C088 and the primary-state constant before the four clearing
 * stores, so the store order D_80146875, g_ScenarioProgress, 0x80146865,
 * 0x80146866, 0x80146867, D_80146874 is the source statement order, and the
 * cue argument is materialised as `addiu $a0,$zero,0x106` in the `jal`
 * delay slot of the final call. No in-image `jal` targets the address. Its
 * 0x60 bytes repeat the store shape of the sibling lift
 * setSubstate0AClearProgressAndSharedStateBytesAndRequestPrimaryState0EScenarioScena0600_801FD170
 * (0x801FD170, scena06/00), which differs in the requested state constant and
 * stops before the cue call, and of
 * clearProgressAndSharedStateBytesThenRequestPrimaryState19ScenarioScena0200_801FD5A8
 * (0x801FD5A8, this target), which clears D_80146875 instead of requesting
 * sub-state 0xA and queues no cue.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState19Substate0AAndQueueCue106ScenarioScena0200_801FD5FC(void) {
  func_8015C088();
  D_80146875 = 0xA;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 0x19;
  func_8015DF18(0x106u);
}
