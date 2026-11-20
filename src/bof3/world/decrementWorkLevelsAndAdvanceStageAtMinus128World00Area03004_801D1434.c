#include "bof3/world/area03004_internal.h"

/* @behavior Subtract 16 from the three work level bytes modulo 256. When the first
 * equals -128 as a signed byte, clear all three, clear flag bit 5, copy byte
 * 0x0B to byte 0x27 and advance byte 3. Otherwise run
 * publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0.
 * @source 0x801D1434
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkLevelsAndAdvanceStageAtMinus128World00Area03004(void) {
  u8* firstLevelWork;
  u8* secondLevelWork;
  u8* work;
  u8* thresholdWork;
  u8* flagsWork;

  firstLevelWork = (u8*)D_1F800044;
  firstLevelWork[0x5Du] = (u8)(firstLevelWork[0x5Du] - 16u);
  secondLevelWork = (u8*)D_1F800044;
  secondLevelWork[0x5Eu] = (u8)(secondLevelWork[0x5Eu] - 16u);
  work = (u8*)D_1F800044;
  work[0x5Fu] = (u8)(work[0x5Fu] - 16u);

  thresholdWork = (u8*)D_1F800044;
  if ((s8)thresholdWork[0x5Du] == -128) {
    thresholdWork[0x5Du] = 0u;
    ((u8*)D_1F800044)[0x5Eu] = 0u;
    ((u8*)D_1F800044)[0x5Fu] = 0u;
    flagsWork = (u8*)D_1F800044;
    flagsWork[0u] = (u8)(flagsWork[0u] & 0xDFu);
    work = (u8*)D_1F800044;
    work[0x27u] = work[0x0Bu];
    work = (u8*)D_1F800044;
    work[3u] = (u8)(work[3u] + 1u);
  } else {
    publishPanelTemplateAndSelectStateWorld00Area03004_801D11C0();
  }
}
