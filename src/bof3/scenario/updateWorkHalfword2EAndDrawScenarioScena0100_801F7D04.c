#include "bof3/bof3.h"

extern u8 modeAt8014933BScenarioScena0100; /* @source 0x8014933B @kind data */
extern u8* scenarioScena0100Work; /* @source 0x1F800044 @kind unknown */
void func_801F7DCC(s32 red, s32 green, s32 blue);
void func_801F7EDC(void);

/* @source 0x801F7D04
 * @behavior Selects drawing work by the unsigned mode byte at 0x8014933B.
 * Mode 1 clears signed work halfword 0x2E and draws a gradient with RGB
 * (64, 90, 255). Mode 2 increments the halfword if it is below 191,
 * reloads it, and draws with RGB (value + 64, 90, max(255 - value * 2, 0)),
 * then calls func_801F7EDC. Other modes do nothing. The work pointer is
 * reloaded after the halfword store. Takes no arguments and returns nothing.
 * @status partial
 * @match 82.00
 * @residual Mode dispatch lacks the original signed range test; red argument scheduling differs.
 */
void updateWorkHalfword2EAndDrawScenarioScena0100_801F7D04(void) {
  s32 value;
  s32 blue;

  switch (modeAt8014933BScenarioScena0100) {
  case 1:
    FIELD_REF(s16, scenarioScena0100Work, 0x2E) = 0;
    func_801F7DCC(64, 90, 255);
    break;
  case 2:
    if (FIELD_REF(s16, scenarioScena0100Work, 0x2E) < 191) {
      FIELD_REF(s16, scenarioScena0100Work, 0x2E)++;
    }
    value = FIELD_REF(s16, scenarioScena0100Work, 0x2E);
    blue = 255 - value * 2;
    if (blue < 0) {
      blue = 0;
    }
    func_801F7DCC(value + 64, 90, blue);
    func_801F7EDC();
    break;
  }
}
