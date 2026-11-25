#include "bof3/bof3.h"
/* @source 0x801FE464 @kind data */
extern u8 sharedFieldSeed0;
/* @source 0x801FE465 @kind data */
extern u8 sharedFieldSeed1;
/* @source 0x801FE466 @kind data */
extern u8 sharedFieldSeed2;
/* @source 0x801FE467 @kind data */
extern u8 sharedFieldSeed3;
/* @source 0x801FE468 @kind data */
extern u8 sharedFieldSeed4;
/* @source 0x801FE469 @kind data */
extern u8 sharedFieldSeed5;
/* @source 0x801FE46A @kind data */
extern u8 sharedFieldSeed6;
/* @source 0x801FE46B @kind data */
extern u8 sharedFieldSeed7;
/* @source 0x80144DF2 @kind data */
extern u8 stateSelectionCounter;
/* @source 0x80144DF5 @kind data */
extern u8 stateSelectionCounter2;
/* @source 0x80144DF8 @kind data */
extern u16 sharedFieldHalfword;
/* @source 0x80144E00 @kind data */
extern u16 sharedFieldHalfword2;
/* @source 0x80144E04 @kind data */
extern u16 sharedFieldHalfword3;
/* @source 0x80144E06 @kind data */
extern u16 sharedFieldHalfword4;
/* @source 0x80144E08 @kind data */
extern u16 sharedFieldHalfword5;
/* @source 0x80144E0A @kind data */
extern u16 sharedFieldHalfword6;
/* @source 0x80144E20 @kind data */
extern u16 stateSharedFieldHalfword;
/* @source 0x80144E24 @kind data */
extern u16 sharedFieldHalfword8;
/* @source 0x80144E26 @kind data */
extern u16 sharedFieldHalfword9;
/* @source 0x80144E28 @kind data */
extern u16 sharedFieldHalfword10;
/* @source 0x80144E2A @kind data */
extern u16 sharedFieldHalfword11;

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
void stateSharedFieldSeed(void) {
  s32 seed0;
  s32 seed6;
  s32 seed7;
  s32 seed1;
  s32 seed2;
  s32 seed3;
  s32 seed4;
  s32 seed5;

  seed0 = sharedFieldSeed0;
  seed6 = sharedFieldSeed6;
  seed7 = sharedFieldSeed7;
  seed1 = sharedFieldSeed1;
  seed2 = sharedFieldSeed2;
  seed3 = sharedFieldSeed3;
  seed4 = sharedFieldSeed4;
  seed5 = sharedFieldSeed5;

  sharedFieldHalfword = seed0;
  stateSelectionCounter = seed6;
  stateSelectionCounter2 = seed7;
  sharedFieldHalfword2 = seed1;
  stateSharedFieldHalfword = seed1;
  sharedFieldHalfword3 = seed2;
  sharedFieldHalfword8 = seed2;
  sharedFieldHalfword4 = seed3;
  sharedFieldHalfword9 = seed3;
  sharedFieldHalfword5 = seed4;
  sharedFieldHalfword10 = seed4;
  sharedFieldHalfword6 = seed5;
  sharedFieldHalfword11 = seed5;
}
