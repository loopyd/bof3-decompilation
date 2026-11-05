#include "bof3/bof3.h"
/* @source 0x801FE464 @kind data */
extern u8 sharedFieldSeed0ScenarioScena0600_801FE464;
/* @source 0x801FE465 @kind data */
extern u8 sharedFieldSeed1ScenarioScena0600_801FE465;
/* @source 0x801FE466 @kind data */
extern u8 sharedFieldSeed2ScenarioScena0600_801FE466;
/* @source 0x801FE467 @kind data */
extern u8 sharedFieldSeed3ScenarioScena0600_801FE467;
/* @source 0x801FE468 @kind data */
extern u8 sharedFieldSeed4ScenarioScena0600_801FE468;
/* @source 0x801FE469 @kind data */
extern u8 sharedFieldSeed5ScenarioScena0600_801FE469;
/* @source 0x801FE46A @kind data */
extern u8 sharedFieldSeed6ScenarioScena0600_801FE46A;
/* @source 0x801FE46B @kind data */
extern u8 sharedFieldSeed7ScenarioScena0600_801FE46B;
/* @source 0x80144DF2 @kind data */
extern u8 sharedFieldByte_80144DF2;
/* @source 0x80144DF5 @kind data */
extern u8 sharedFieldByte_80144DF5;
/* @source 0x80144DF8 @kind data */
extern u16 sharedFieldHalfword_80144DF8;
/* @source 0x80144E00 @kind data */
extern u16 sharedFieldHalfword_80144E00;
/* @source 0x80144E04 @kind data */
extern u16 sharedFieldHalfword_80144E04;
/* @source 0x80144E06 @kind data */
extern u16 sharedFieldHalfword_80144E06;
/* @source 0x80144E08 @kind data */
extern u16 sharedFieldHalfword_80144E08;
/* @source 0x80144E0A @kind data */
extern u16 sharedFieldHalfword_80144E0A;
/* @source 0x80144E20 @kind data */
extern u16 sharedFieldHalfword_80144E20;
/* @source 0x80144E24 @kind data */
extern u16 sharedFieldHalfword_80144E24;
/* @source 0x80144E26 @kind data */
extern u16 sharedFieldHalfword_80144E26;
/* @source 0x80144E28 @kind data */
extern u16 sharedFieldHalfword_80144E28;
/* @source 0x80144E2A @kind data */
extern u16 sharedFieldHalfword_80144E2A;

/* @source 0x801FD29C
 * @behavior Load the eight overlay seed bytes at 0x801FE464..0x801FE46B
 * before writing shared RAM fields. Seed 0 initializes halfword 0x80144DF8;
 * seeds 6 and 7 initialize bytes 0x80144DF2 and 0x80144DF5. Seeds 1 through
 * 5 initialize halfwords 0x80144E00, 0x80144E04, 0x80144E06, 0x80144E08,
 * 0x80144E0A and their mirrors at +0x20. Takes no arguments and returns
 * nothing. The field roles beyond this initialization are unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSharedFieldsScenarioScena0600_801FD29C(void) {
  s32 seed0;
  s32 seed6;
  s32 seed7;
  s32 seed1;
  s32 seed2;
  s32 seed3;
  s32 seed4;
  s32 seed5;

  seed0 = sharedFieldSeed0ScenarioScena0600_801FE464;
  seed6 = sharedFieldSeed6ScenarioScena0600_801FE46A;
  seed7 = sharedFieldSeed7ScenarioScena0600_801FE46B;
  seed1 = sharedFieldSeed1ScenarioScena0600_801FE465;
  seed2 = sharedFieldSeed2ScenarioScena0600_801FE466;
  seed3 = sharedFieldSeed3ScenarioScena0600_801FE467;
  seed4 = sharedFieldSeed4ScenarioScena0600_801FE468;
  seed5 = sharedFieldSeed5ScenarioScena0600_801FE469;

  sharedFieldHalfword_80144DF8 = seed0;
  sharedFieldByte_80144DF2 = seed6;
  sharedFieldByte_80144DF5 = seed7;
  sharedFieldHalfword_80144E00 = seed1;
  sharedFieldHalfword_80144E20 = seed1;
  sharedFieldHalfword_80144E04 = seed2;
  sharedFieldHalfword_80144E24 = seed2;
  sharedFieldHalfword_80144E06 = seed3;
  sharedFieldHalfword_80144E26 = seed3;
  sharedFieldHalfword_80144E08 = seed4;
  sharedFieldHalfword_80144E28 = seed4;
  sharedFieldHalfword_80144E0A = seed5;
  sharedFieldHalfword_80144E2A = seed5;
}
