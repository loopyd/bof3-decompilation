#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
void func_801AFE18(void* arg0);
void func_801AFF04(const void* arg0, void* arg1);
void func_801F8108(s32 arg0, s32 arg1);
void func_801F823C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4(u16 arg0);

/* @source 0x801F8534
 * @behavior Overlay drawing step for the scratchpad work object published at
 * 0x1F800044: it first fills the 0x20-byte projection scratch buffer through
 * func_801AFE18, then projects the work object's cursor words at offsets
 * 0x34/0x38/0x3C through the shared projector func_801AFF04 into a local
 * eight-byte screen pair, and stores that pair into the work object's halfwords
 * at offsets 0x2E (projected x) and 0x30 (projected y). It then calls
 * func_801F8108 with the signed halfword at 0x2E and the signed projected y,
 * which stamps two 0x18-byte cells at the projected position; shifts the
 * overlay angle table at 0x801FE9EC one entry through
 * shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4 with the work
 * object's unsigned halfword at 0x58 as the new head value; and finally calls
 * func_801F823C with the work object's signed halfwords 0x2E, 0x30, 0x32 and
 * 0x2C, which lays out its cells around the projected pair from that shifted
 * table. The scratchpad work pointer is reloaded once per call group, so the
 * stores and reload in the second group share one pointer register. Takes no
 * arguments and returns nothing. The four callees and the work halfwords'
 * wider roles remain unknown, so they stay address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectWorkCursorStoreScreenPairAndDrawSpritePairAndRingScenarioScena0800_801F8534(void) {
  VECTOR point;
  u16 sxy[4];
  u8 scratch[0x20];
  u8* work;
  u8* ringWork;
  u16 screenY;

  func_801AFE18(scratch);
  work = scenarioScena0800Work;
  point.vx = *(s32*)(work + 0x34);
  point.vy = *(s32*)(work + 0x38);
  point.vz = *(s32*)(work + 0x3C);
  func_801AFF04(&point, sxy);
  work = scenarioScena0800Work;
  *(u16*)(work + 0x2E) = sxy[0];
  screenY = sxy[1];
  *(u16*)(work + 0x30) = screenY;
  func_801F8108(*(s16*)(work + 0x2E), (s16)screenY);
  ringWork = scenarioScena0800Work;
  shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4(
      *(u16*)(ringWork + 0x58));
  ringWork = scenarioScena0800Work;
  func_801F823C(*(s16*)(ringWork + 0x2E), *(s16*)(ringWork + 0x30),
                *(s16*)(ringWork + 0x32), *(s16*)(ringWork + 0x2C));
}
