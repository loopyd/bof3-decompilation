#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void emitTwoDrawAreaMovesForCursorCellPairScenarioScena0800_801F8108(s32 arg0, s32 arg1);
void func_801F85F0(s32 arg0, s32 arg1, s16 arg2);

/* @source 0x801F7688
 * @behavior Overlay work-state handler, entry 10 of the twelve-entry handler
 * sub-table at 0x801FE8A0 (entry 13 of the 51-entry run at 0x801FE894) that the
 * 0x801FE8A0 dispatcher selects with unsigned byte 1 of the scratchpad work
 * object: it fills the 0x20-byte projection scratch through func_801AFE18,
 * projects the work object cursor words at 0x34/0x38/0x3C through func_801AFF04
 * into a local screen pair, and stores that pair into the work object halfwords
 * 0x2E (projected x) and 0x30 (projected y). It seeds halfword 0x32 with 0x140
 * and the countdown halfword 0x5A with 0x14, and increments work byte 1 modulo
 * 256 so the next frame dispatches entry 11 at 0x801F774C, which subtracts 16
 * from halfword 0x32 each frame while that countdown runs. It then moves the
 * two 0x18-byte cells through
 * emitTwoDrawAreaMovesForCursorCellPairScenarioScena0800_801F8108 with the
 * signed halfwords 0x2E and 0x30, and lays out the ring through func_801F85F0
 * with the signed halfwords 0x2E, 0x30 and 0x32, reloading the scratchpad work
 * pointer once per call group.
 * Takes no arguments and returns nothing. The callees' wider roles and the work
 * halfword meanings downstream of this seed remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkCursorStoreScreenPairSeedHalfword32AndCountdown5AThenDrawAndAdvanceScenarioScena0800_801F7688(void) {
  VECTOR point;
  u16 sxy[4];
  u8 scratch[0x20];
  u8* pointWork;
  u8* work;
  u8* drawWork;
  u8* ringWork;
  u16 screenY;

  func_801AFE18(scratch);
  pointWork = scenarioScena0800Work;
  point.vx = *(s32*)(pointWork + 0x34);
  point.vy = *(s32*)(pointWork + 0x38);
  point.vz = *(s32*)(pointWork + 0x3C);
  func_801AFF04(&point, sxy);
  work = scenarioScena0800Work;
  *(u16*)(work + 0x2E) = sxy[0];
  screenY = sxy[1];
  *(u16*)(work + 0x32) = 0x140;
  work[1]++;
  drawWork = scenarioScena0800Work;
  *(u16*)(work + 0x5A) = 0x14;
  *(u16*)(work + 0x30) = screenY;
  emitTwoDrawAreaMovesForCursorCellPairScenarioScena0800_801F8108(
      *(s16*)(drawWork + 0x2E), *(s16*)(drawWork + 0x30));
  ringWork = scenarioScena0800Work;
  func_801F85F0(*(s16*)(ringWork + 0x2E), *(s16*)(ringWork + 0x30),
                *(s16*)(ringWork + 0x32));
}
