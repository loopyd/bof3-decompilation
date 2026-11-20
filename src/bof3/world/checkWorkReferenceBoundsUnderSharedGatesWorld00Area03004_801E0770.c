#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E0770
 * @behavior Require shared bytes 0x80143FC9 = 4, 0x801E3208 = 0xFF, 0x80144199 = 0 and
 * 0x80145E92 = 3. Test the computed signed x and y differences against 0x7FFF
 * and the signed-halfword counter difference strictly below 0x400. Preserve
 * the original signed arithmetic and sampled values.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 checkWorkReferenceBoundsUnderSharedGatesWorld00Area03004(void) {
  Area030WorkRecord* workRecord;
  s32 matchesBounds;
  s32 counterDifference;
  s32 sample;
  s32 test;
  s32 positionLimit;

  matchesBounds = 0;
  do {
    sample = D_80143FC9;
    test = 4;
    if (sample != test) {
      break;
    }
    sample = D_801E3208;
    test = 0xFF;
    if (sample != test) {
      break;
    }
    test = D_80144199[0];
    if (test != 0) {
      break;
    }
    sample = D_80145E92;
    test = 3;
    if (sample != test) {
      break;
    }
    do {
      positionLimit = 0x7FFF;
    } while (0);
    workRecord = (Area030WorkRecord*)D_1F800044;
    sample = D_80143FFC;
    test = workRecord->x_34;
    test -= sample;
    if (test < 0) {
      test = -test;
    }
    test = positionLimit < test;
    if (test != 0) {
      break;
    }
    test = workRecord->y_38;
    sample = D_80144000;
    test -= sample;
    if (test < 0) {
      test = -test;
    }
    test = positionLimit < test;
    if (test != 0) {
      break;
    }
    counterDifference = workRecord->counter_3E;
    test = D_80144006;
    counterDifference -= test;
    if (counterDifference < 0) {
      counterDifference = -counterDifference;
    }
    matchesBounds = counterDifference < 0x400;
  } while (0);
  return matchesBounds;
}
