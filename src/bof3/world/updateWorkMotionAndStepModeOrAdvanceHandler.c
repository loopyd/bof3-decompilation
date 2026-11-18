#include "bof3/world/area03004_internal.h"

extern u8 firstFrontEndEntryFlags; /* @source 0x80143FC8 @kind bss */

extern s16 workOffsetReferenceState; /* @source 0x8014423C @kind unknown */
extern s32 func_801E02D4(u32 arg0, u32 arg1);

/* @source 0x801DF334
 * @behavior updates the AREA030 work record's signed offset, motion words,
 * range and step mode using its countdown, selected scale record and random
 * draws. Threshold transitions publish the shared mode bytes and advance
 * handler byte 1. Other paths update the motion words and select the
 * two-byte step-mode row from work byte 8. Preserves the separate
 * scratch-pointer captures, signed clamps and guarded division.
 * @status partial
 * @match 84.57
 * @residual Frame size, bound copies, value allocation and division expansion
 * still differ. The original 3188-byte interval remains
 * retained until exact reconstruction.
 */
void updateWorkMotionAndStepModeOrAdvanceHandler(void) {
  u8* upperThresholdWork;
  u8* upperMotionWork;
  u8* transitionWork;
  u8* lowerThresholdWork;
  u8* upperMode;
  u8* lowerMode;
  u8* entryMode;
  u8* work;
  u32* outwardPositionOutput;
  u32* inwardPositionOutput;
  u8* outwardConditionWork;
  u8* outwardPositionWork;
  u8* outwardVelocityWork;
  u8* inwardConditionWork;
  u8* inwardPositionWork;
  u8* inwardModeWork;
  u8* countdownGuardWork;
  u8* accumulatedAngleWork;
  u8* boundedAngleWork;
  u8* stepSelectionWork;
  u8* requestSelectionWork;
  u8* activeMotionWork;
  u8* activeModeWork;
  u8* inactiveGuardWork;
  u8* inactiveMotionWork;
  u8* nextCountdownWork;
  u8* seedMotionWork;
  u8* negativeMotionWork;
  u8* clampMotionWork;
  u8* relativeAngleWork;
  u8* clearMotionWork;
  u8* capabilityWork;
  u8* directionMotionWork;
  u8* stationaryMotionWork;
  u8* stepModeWork;
  u8* activeStateWork;
  u8* chanceGuardWork;
  u8* chanceSoundWork;
  u8* chanceFlagWork;
  u8* chanceStepWork;
  u8* chanceIdleWork;
  u8* chanceTimerWork;
  u8* chanceMotionWork;
  u8* chanceStateWork;
  u8* soundWork;
  u8* flagWork;
  u8* stepWork;
  u8* requestWork;
  u8* timerWork;
  u8* idleWork;
  u8* motionWork;
  u8* countWork;
  u8* angleWork;
  u8* scaleBytes;
  Area030Range* classificationRange;
  Area030Range* retryRange;
  Area030Range* deltaRange;
  Area030Range* activeRange;
  Area030Range* inactiveRange;
  Area030Range* countdownRange;
  s32 reference;
  s32 inwardReference;
  s32 outwardReference;
  s32 outwardAngle;
  s32 spread;
  s32 lower;
  s32 upper;
  s32 angle;
  s8 sampledAngle;
  s8 sampledMode3Angle;
  u8 retainedMode3Angle;
  u8 selectedMode3Angle;
  u8 retainedAngle;
  u8 selectedAngle;
  s32 seedAngle;
  u8 level;
  s32 roll;
  s32 value;
  u32 mode3Motion;
  u32 accumulatedMotion;
  s32 sampledMotion;
  s32 positiveDirection;
  s32 selectedMotion;
  s32 limit;
  s32 initialCurrent;
  s32 retryCurrent;
  s32 countdownCurrent;
  s32 delta;
  s32 position;
  s32 inwardStepPosition;
  s32 publishedInwardPosition;
  s32 publishedOutwardPosition;
  s32 velocity;
  s32 selectedInwardVelocity;
  s32 classificationVelocity;
  s32 initialOutwardVelocity;
  s32 outwardTimer;
  s32 timerValue;
  s32 outwardTimerNegative;
  s32 inwardTimerNegative;
  s32 numerator;
  s32 denominator;
  u8 count;
  u8 threshold;

  entryMode = (u8*)D_801E320C;
  reference = ((u8*)&workOffsetReferenceState)[0];
  spread = entryMode[3] * 4;
  work = D_1F800044;
  do {
    lower = reference - spread - 0x0C;
    upper = reference + spread + 0x0C;
  } while (0);
  scaleBytes = (u8*)&D_801E2AB8[work[6]];

  if (FIELD_REF(s32, work, 0x38) <= 0x8000) {
    D_8014421C = -7;
    D_80143FC9 = 0;
    D_80145E92 = 6;
    D_80145E93 = 0;
    work[1] = (u8)(work[1] + 1);
    return;
  }

  if (work[4] == 3) {
    if (func_8014D978() != 0) {
      soundWork = D_1F800044;
      func_8014DD3C(D_801E2AB8[soundWork[6]].entry_10);

      flagWork = D_1F800044;
      flagWork[0] = (u8)(flagWork[0] | 0x20);

      stepWork = D_1F800044;
      stepWork[0x2A] = D_801E2330[stepWork[8]].byte_01;

      requestWork = D_1F800044;
      func_8014D6B8(D_801E2330[requestWork[8]].request_00);

      timerWork = D_1F800044;
      FIELD_REF(s16, timerWork, 0x3E) = -0x20;
      timerWork[4] = 0;

      idleWork = D_1F800044;
      idleWork[0x48] = 2;

      motionWork = D_1F800044;
      FIELD_REF(s32, motionWork, 0x10) = 0;

      firstFrontEndEntryFlags = (u8)(firstFrontEndEntryFlags & 0xBF);
    }

    countWork = D_1F800044;
    if (FIELD_REF(u16, countWork, 0x58) >= 0x15) {
      mode3Motion = FIELD_REF(u32, countWork, 0x10);
      countWork[0x0A] = (u8)(countWork[0x0A] + mode3Motion);

      angleWork = D_1F800044;
      sampledMode3Angle = (s8)angleWork[0x0A];
      if (sampledMode3Angle >= -0x30) {
        retainedMode3Angle = (u8)sampledMode3Angle;
        if (sampledMode3Angle < 0x31) {
          selectedMode3Angle = retainedMode3Angle;
        } else {
          selectedMode3Angle = 0x30;
        }
      } else {
        selectedMode3Angle = 0xD0;
      }
      angleWork[0x0A] = selectedMode3Angle;
    }
    return;
  }

  if (D_80144199_BYTE == 0) {
    angle = (s8)work[0x0A];
    if (angle > (s8)upper) {
      work[0x0B] = (u8)(work[0x0B] + 1);

      upperThresholdWork = D_1F800044;
      upperMode = (u8*)D_801E320C;
      count = upperThresholdWork[0x0B];
      threshold = upperMode[8];
      if (count >= threshold) {
        upperThresholdWork[0x0B] = 0;
        if (D_8014421C < 0) {
          upperMotionWork = D_1F800044;
          if (FIELD_REF(s32, upperMotionWork, 0x10) > 0) {
            if ((u8)func_801E02D4((u8)upper, 1) != 0) {
              s32 transitionStatus;
              u8 transitionMode;
              do {
                transitionStatus = -7;
              } while (0);
              transitionWork = D_1F800044;
              do {
                D_8014421C = transitionStatus;
                transitionMode = 6;
              } while (0);
              D_80143FC9 = 0;
              D_80145E92 = transitionMode;
              D_80145E93 = 0;
              transitionWork[1] = (u8)(transitionWork[1] + 1);
              return;
            }
          }
        }
      }
    } else if (angle < (s8)lower) {
      work[0x0B] = (u8)(work[0x0B] + 1);

      lowerThresholdWork = D_1F800044;
      lowerMode = (u8*)D_801E320C;
      count = lowerThresholdWork[0x0B];
      threshold = lowerMode[9];
      if (count >= threshold) {
        lowerThresholdWork[0x0B] = 0;
        if ((u8)func_801E02D4((u8)lower, 2) != 0) {
          u8 transitionMode;
          do {
            transitionMode = 7;
          } while (0);
          transitionWork = D_1F800044;
          do {
            D_80145E92 = transitionMode;
          } while (0);
          D_80145E93 = 0;
          D_80143FC9 = 0;
          transitionWork[1] = (u8)(transitionWork[1] + 1);
          return;
        }
      }
    } else {
      work[0x0B] = 0;
    }
  }

  work = D_1F800044;
  if (work[9] == 0) {
    classificationRange = D_80146884;
    limit = classificationRange->limit_8C;
    initialCurrent = classificationRange->value_8E;
    reference = limit >> 2;

    if (initialCurrent < reference) {
      level = 3;
    } else if (initialCurrent < (limit >> 1)) {
      level = 2;
    } else {
      level = initialCurrent < reference * 3;
    }

    roll = rand();
    if ((roll & 0x0F) < scaleBytes[level * 4]) {
      chanceGuardWork = D_1F800044;
      timerValue = FIELD_REF(s16, chanceGuardWork, 0x3E);
      if (timerValue >= -0x3F &&
          (s8)chanceGuardWork[0x0A] > 0) {
        D_80143FC8[0].flags_00 =
            (u8)(D_80143FC8[0].flags_00 | 0x40);
        func_8015DF18(0x206);

        chanceSoundWork = D_1F800044;
        func_8014DD3C(D_801E2AB8[chanceSoundWork[6]].entry_12);

        chanceFlagWork = D_1F800044;
        chanceFlagWork[0] = (u8)(chanceFlagWork[0] & 0xDF);

        chanceStepWork = D_1F800044;
        chanceStepWork[0x2A] = 0;

        chanceIdleWork = D_1F800044;
        chanceIdleWork[0x48] = 0;

        chanceTimerWork = D_1F800044;
        FIELD_REF(s16, chanceTimerWork, 0x3E) = 0;
        func_8014D6B8(0);

        chanceMotionWork = D_1F800044;
        FIELD_REF(s32, chanceMotionWork, 0x10) = -3;
        func_801E026C(-5);

        chanceStateWork = D_1F800044;
        chanceStateWork[4] = 3;
        return;
      }
    }

    roll = rand();
    scaleBytes += level * 4;
    if ((roll & 0x0F) < scaleBytes[1]) {
      retryRange = D_80146884;
      limit = retryRange->limit_8C;
      retryCurrent = retryRange->value_8E;

      if (retryCurrent > (limit >> 4)) {
        roll = rand();
        seedMotionWork = D_1F800044;
        value = D_801E2AB8[seedMotionWork[6]].unk_1C[1] & 7;
        seedAngle = (s8)seedMotionWork[0x0A];
        FIELD_REF(s32, seedMotionWork, 0x10) = (roll & value) + 1;

        if (seedAngle < 0) {
          roll = rand();
          negativeMotionWork = D_1F800044;
          FIELD_REF(u32, negativeMotionWork, 0x10) =
              FIELD_REF(u32, negativeMotionWork, 0x10) + (u32)(roll & 1);
        }

        clampMotionWork = D_1F800044;
        sampledMotion = FIELD_REF(s32, clampMotionWork, 0x10);
        if (sampledMotion >= 0) {
          if (sampledMotion < 6) {
            selectedMotion = sampledMotion;
          } else {
            selectedMotion = 5;
          }
        } else {
          selectedMotion = 0;
        }
        FIELD_REF(s32, clampMotionWork, 0x10) = selectedMotion;

        if (D_8014421C < 0) {
          relativeAngleWork = D_1F800044;
          value = (s8)relativeAngleWork[0x0A] - workOffsetReferenceState;
          if (value < -0x30) {
            deltaRange = D_80146884;
            FIELD_REF(u8, deltaRange, 0x75) = 0xFD;
          } else if (value < -0x18) {
            deltaRange = D_80146884;
            FIELD_REF(u8, deltaRange, 0x75) = 0xFE;
          } else {
            deltaRange = D_80146884;
            FIELD_REF(u8, deltaRange, 0x75) = 0xFF;
          }
        }
      } else {
        clearMotionWork = D_1F800044;
        FIELD_REF(s32, clearMotionWork, 0x10) = 0;
        FIELD_REF(u8, retryRange, 0x75) = 0;
      }

      capabilityWork = D_1F800044;
      if (D_801E2AB8[capabilityWork[6]].unk_20[0] != 0) {
        roll = rand();
        if ((roll & 1) != 0) {
          roll = rand();
          if ((roll & 1) != 0) {
            do {
              positiveDirection = 0x1000;
            } while (0);
            directionMotionWork = D_1F800044;
            FIELD_REF(s32, directionMotionWork, 0x0C) = positiveDirection;
          } else {
            directionMotionWork = D_1F800044;
            FIELD_REF(s32, directionMotionWork, 0x0C) = -0x1000;
          }
        } else {
          stationaryMotionWork = D_1F800044;
          FIELD_REF(s32, stationaryMotionWork, 0x0C) = 0;
        }
      } else {
        FIELD_REF(s32, capabilityWork, 0x0C) = 0;
      }

      stepModeWork = D_1F800044;
      stepModeWork[8] = 1;
      activeStateWork = D_1F800044;
      activeStateWork[4] = 0;
    } else {
      roll = rand();
      if ((roll & 0x0F) < scaleBytes[2]) {
        activeMotionWork = D_1F800044;
        FIELD_REF(s32, activeMotionWork, 0x0C) = 0;
        FIELD_REF(s32, activeMotionWork, 0x10) = 0;

        activeRange = D_80146884;
        FIELD_REF(u8, activeRange, 0x75) = 1;

        activeModeWork = D_1F800044;
        activeModeWork[4] = 1;
      } else {
        inactiveGuardWork = D_1F800044;
        if (inactiveGuardWork[0x0A] != 0) {
          inactiveRange = D_80146884;
          FIELD_REF(u8, inactiveRange, 0x75) = 0;

          inactiveMotionWork = D_1F800044;
          FIELD_REF(s32, inactiveMotionWork, 0x10) = -1;
          FIELD_REF(s32, inactiveMotionWork, 0x0C) = 0;
          inactiveMotionWork[4] = 2;
        }
      }
    }

    roll = rand();
    nextCountdownWork = D_1F800044;
    nextCountdownWork[9] = (u8)((8 - level) * ((roll & 3) + 1));
  } else {
    work[9] = (u8)(work[9] - 1);

    countdownRange = D_80146884;
    delta = FIELD_REF(s8, countdownRange, 0x75);
    if (delta > 0) {
      if ((D_80143E6C & 0x0F) == 0) {
        countdownGuardWork = D_1F800044;
        if (FIELD_REF(s32, countdownGuardWork, 0x10) >= 0) {
          limit = countdownRange->limit_8C;
          countdownCurrent = countdownRange->value_8E;
          if (countdownCurrent < (limit >> 1)) {
            func_801E026C((s8)delta);
          }
        }
      }
    } else if ((D_80143E6C & 1) != 0) {
      func_801E026C((s8)delta);
    }

    accumulatedAngleWork = D_1F800044;
    accumulatedMotion = FIELD_REF(u32, accumulatedAngleWork, 0x10) +
                        FIELD_REF(u32, accumulatedAngleWork, 0x1C);
    accumulatedAngleWork[0x0A] = (u8)(accumulatedAngleWork[0x0A] + accumulatedMotion);
  }

  boundedAngleWork = D_1F800044;
  sampledAngle = (s8)boundedAngleWork[0x0A];
  if (sampledAngle >= -0x30) {
    retainedAngle = (u8)sampledAngle;
    if (sampledAngle < 0x31) {
      selectedAngle = retainedAngle;
    } else {
      selectedAngle = 0x30;
    }
  } else {
    selectedAngle = 0xD0;
  }
  boundedAngleWork[0x0A] = selectedAngle;

  if (D_80149332 == 0) {
    D_8014932E = 0;
  }

  outwardConditionWork = D_1F800044;
  outwardReference = workOffsetReferenceState;
  outwardAngle = (s8)outwardConditionWork[0x0A];
  if (outwardAngle > outwardReference && FIELD_REF(s32, outwardConditionWork, 0x10) > 0) {
    position = FIELD_REF(s32, outwardConditionWork, 0x38);
    outwardTimer = FIELD_REF(s16, outwardConditionWork, 0x3E);
    do {
      FIELD_REF(u32, outwardConditionWork, 0x38) = (u32)position - 0x1000u;
    } while (0);
    outwardTimerNegative = outwardTimer < 0;
    timerValue = outwardTimer;
    if (outwardTimerNegative) {
      timerValue += 8;
      FIELD_REF(u16, outwardConditionWork, 0x3E) = (u16)timerValue;
    }

    outwardPositionWork = D_1F800044;
    publishedOutwardPosition = FIELD_REF(s32, outwardPositionWork, 0x38);
    if (publishedOutwardPosition > 0x150000) {
      D_8014932E = 0x10;
      outwardPositionOutput = &D_8014930C;
      *outwardPositionOutput = (u32)publishedOutwardPosition;
      if (publishedOutwardPosition > 0x3C0000) {
        *outwardPositionOutput = 0x3C0000;
      }
    }

    outwardVelocityWork = D_1F800044;
    initialOutwardVelocity = FIELD_REF(s32, outwardVelocityWork, 0x0C);
    if (initialOutwardVelocity < 0) {
      if (FIELD_REF(s32, outwardVelocityWork, 0x34) <= 0x60000) {
        FIELD_REF(u32, outwardVelocityWork, 0x0C) = 0u - (u32)initialOutwardVelocity;
        outwardVelocityWork = D_1F800044;
      }
    } else {
      if (initialOutwardVelocity > 0) {
        if (FIELD_REF(s32, outwardVelocityWork, 0x34) > 0x13FFFF) {
          FIELD_REF(u32, outwardVelocityWork, 0x0C) = 0u - (u32)initialOutwardVelocity;
        }
      } else {
        FIELD_REF(s32, outwardVelocityWork, 0x0C) = 0;
      }
      outwardVelocityWork = D_1F800044;
    }

    FIELD_REF(u32, outwardVelocityWork, 0x34) =
        FIELD_REF(u32, outwardVelocityWork, 0x34) +
        FIELD_REF(u32, outwardVelocityWork, 0x0C);
    classificationVelocity = FIELD_REF(s32, outwardVelocityWork, 0x0C);
    if (classificationVelocity < -0x800) {
      outwardVelocityWork[8] = 0;
    } else if (classificationVelocity > 0x800) {
      outwardVelocityWork[8] = 2;
    } else {
      outwardVelocityWork[8] = 1;
    }
  } else {
    inwardConditionWork = D_1F800044;
    inwardReference = workOffsetReferenceState;
    if ((s8)inwardConditionWork[0x0A] < inwardReference && D_8014421C < 0) {
      do {
        denominator = 0x3E - FIELD_REF(s16, inwardConditionWork, 0x3A);
      } while (0);
      numerator =
          (s32)(0xF0000u - FIELD_REF(u32, inwardConditionWork, 0x34));
      numerator >>= 4;
      if (denominator != 0) {
        denominator = numerator / denominator;
        FIELD_REF(s32, inwardConditionWork, 0x0C) = denominator;
        if (denominator >= -0x1000) {
          if (denominator < 0x1001) {
            selectedInwardVelocity = denominator;
          } else {
            selectedInwardVelocity = 0x1000;
          }
        } else {
          selectedInwardVelocity = -0x1000;
        }
        FIELD_REF(s32, inwardConditionWork, 0x0C) = selectedInwardVelocity;
      } else {
        FIELD_REF(s32, inwardConditionWork, 0x0C) = 0;
      }

      inwardConditionWork = D_1F800044;
      inwardStepPosition = FIELD_REF(s32, inwardConditionWork, 0x34);
      velocity = FIELD_REF(s32, inwardConditionWork, 0x0C);
      do {
        denominator = FIELD_REF(s16, inwardConditionWork, 0x3E);
        FIELD_REF(u32, inwardConditionWork, 0x34) = (u32)inwardStepPosition + (u32)velocity;
      } while (0);
      inwardTimerNegative = denominator < 0;
      timerValue = denominator;
      if (inwardTimerNegative) {
        timerValue += 8;
        FIELD_REF(s16, inwardConditionWork, 0x3E) = (s16)timerValue;
      }

      inwardPositionWork = D_1F800044;
      publishedInwardPosition = (s32)(FIELD_REF(u32, inwardPositionWork, 0x38) + 0x1000u);
      FIELD_REF(s32, inwardPositionWork, 0x38) = publishedInwardPosition;
      if (publishedInwardPosition > 0x150000) {
        u16 indicatorValue;
        s32 maximumInwardPosition;
        do {
          inwardPositionOutput = &D_8014930C;
        } while (0);
        do {
          indicatorValue = 0x10;
          maximumInwardPosition = 0x3C0000;
        } while (0);
        D_8014932E = indicatorValue;
        *inwardPositionOutput = (u32)publishedInwardPosition;
        if (publishedInwardPosition > maximumInwardPosition) {
          *inwardPositionOutput = (u32)maximumInwardPosition;
        }
      }

      inwardModeWork = D_1F800044;
      if (inwardModeWork[4] != 0) {
        inwardModeWork[8] = 5;
      } else {
        inwardModeWork[8] = 1;
      }
    }
  }

  stepSelectionWork = D_1F800044;
  stepSelectionWork[0x2A] = D_801E2330[stepSelectionWork[8]].byte_01;

  requestSelectionWork = D_1F800044;
  func_8014D8D4(D_801E2330[requestSelectionWork[8]].request_00);
  func_8014D978();
}
