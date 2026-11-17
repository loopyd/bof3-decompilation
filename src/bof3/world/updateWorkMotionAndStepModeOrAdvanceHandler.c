#include "bof3/world/area03004_internal.h"

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
 * @match 47.68
 * @residual Complete native bytes still differ. The original 3188-byte
 * interval remains retained until exact reconstruction.
 */
void updateWorkMotionAndStepModeOrAdvanceHandler(void) {
  u8* mode;
  u8* work;
  u8* chanceRow;
  Area030ScaleRecord* scale;
  Area030Range* range;
  s32 reference;
  s32 spread;
  s32 lower;
  s32 upper;
  s32 angle;
  s32 level;
  s32 roll;
  s32 value;
  s32 quarter;
  s32 limit;
  s32 current;
  s32 delta;
  s32 position;
  s32 velocity;
  s32 timer;
  s32 numerator;
  s32 denominator;
  u8 count;
  u8 threshold;

  mode = (u8*)D_801E320C;
  reference = ((u8*)&workOffsetReferenceState)[0];
  spread = mode[3] * 4;
  work = D_1F800044;
  lower = reference - spread - 0x0C;
  upper = reference + spread + 0x0C;
  scale = &D_801E2AB8[work[6]];

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
      work = D_1F800044;
      func_8014DD3C(D_801E2AB8[work[6]].entry_10);

      work = D_1F800044;
      work[0] = (u8)(work[0] | 0x20);

      work = D_1F800044;
      work[0x2A] = D_801E2330[work[8]].byte_01;

      work = D_1F800044;
      func_8014D6B8(D_801E2330[work[8]].request_00);

      work = D_1F800044;
      FIELD_REF(s16, work, 0x3E) = -0x20;
      work[4] = 0;

      work = D_1F800044;
      work[0x48] = 2;

      work = D_1F800044;
      FIELD_REF(s32, work, 0x10) = 0;

      D_80143FC8[0].flags_00 =
          (u8)(D_80143FC8[0].flags_00 & 0xBF);
    }

    work = D_1F800044;
    if (FIELD_REF(u16, work, 0x58) >= 0x15) {
      value = FIELD_REF(s32, work, 0x10);
      work[0x0A] = (u8)(work[0x0A] + (u32)value);

      work = D_1F800044;
      angle = (s8)work[0x0A];
      if (angle < -0x30) {
        angle = -0x30;
      } else if (angle > 0x30) {
        angle = 0x30;
      }
      work[0x0A] = (u8)angle;
    }
    return;
  }

  if (D_80144199_BYTE == 0) {
    angle = (s8)work[0x0A];
    if (angle > (s8)upper) {
      work[0x0B] = (u8)(work[0x0B] + 1);

      work = D_1F800044;
      mode = (u8*)D_801E320C;
      count = work[0x0B];
      threshold = mode[8];
      if (count >= threshold) {
        work[0x0B] = 0;
        if (D_8014421C < 0) {
          work = D_1F800044;
          if (FIELD_REF(s32, work, 0x10) > 0) {
            if ((u8)func_801E02D4((u8)upper, 1) != 0) {
              work = D_1F800044;
              D_8014421C = -7;
              D_80143FC9 = 0;
              D_80145E92 = 6;
              D_80145E93 = 0;
              work[1] = (u8)(work[1] + 1);
              return;
            }
          }
        }
      }
    } else if (angle < (s8)lower) {
      work[0x0B] = (u8)(work[0x0B] + 1);

      work = D_1F800044;
      mode = (u8*)D_801E320C;
      count = work[0x0B];
      threshold = mode[9];
      if (count >= threshold) {
        work[0x0B] = 0;
        if ((u8)func_801E02D4((u8)lower, 2) != 0) {
          work = D_1F800044;
          D_80145E92 = 7;
          D_80145E93 = 0;
          D_80143FC9 = 0;
          work[1] = (u8)(work[1] + 1);
          return;
        }
      }
    } else {
      work[0x0B] = 0;
    }
  }

  work = D_1F800044;
  if (work[9] == 0) {
    range = D_80146884;
    limit = range->limit_8C;
    current = range->value_8E;
    quarter = limit >> 2;

    if (current < quarter) {
      level = 3;
    } else if (current < (limit >> 1)) {
      level = 2;
    } else {
      level = current < quarter * 3;
    }

    chanceRow = &scale->unk_00[level * 4];
    roll = rand();
    if ((roll & 0x0F) < chanceRow[0]) {
      work = D_1F800044;
      if (FIELD_REF(s16, work, 0x3E) >= -0x3F &&
          (s8)work[0x0A] > 0) {
        D_80143FC8[0].flags_00 =
            (u8)(D_80143FC8[0].flags_00 | 0x40);
        func_8015DF18(0x206);

        work = D_1F800044;
        func_8014DD3C(D_801E2AB8[work[6]].entry_12);

        work = D_1F800044;
        work[0] = (u8)(work[0] & 0xDF);

        work = D_1F800044;
        work[0x2A] = 0;

        work = D_1F800044;
        work[0x48] = 0;

        work = D_1F800044;
        FIELD_REF(s16, work, 0x3E) = 0;
        func_8014D6B8(0);

        work = D_1F800044;
        FIELD_REF(s32, work, 0x10) = -3;
        func_801E026C(-5);

        work = D_1F800044;
        work[4] = 3;
        return;
      }
    }

    roll = rand();
    if ((roll & 0x0F) < chanceRow[1]) {
      range = D_80146884;
      limit = range->limit_8C;
      current = range->value_8E;

      if (current > (limit >> 4)) {
        roll = rand();
        work = D_1F800044;
        value = D_801E2AB8[work[6]].unk_1C[1] & 7;
        angle = (s8)work[0x0A];
        FIELD_REF(s32, work, 0x10) = (roll & value) + 1;

        if (angle < 0) {
          roll = rand();
          work = D_1F800044;
          FIELD_REF(u32, work, 0x10) =
              FIELD_REF(u32, work, 0x10) + (u32)(roll & 1);
        }

        work = D_1F800044;
        value = FIELD_REF(s32, work, 0x10);
        if (value < 0) {
          value = 0;
        } else if (value > 5) {
          value = 5;
        }
        FIELD_REF(s32, work, 0x10) = value;

        if (D_8014421C < 0) {
          work = D_1F800044;
          value = (s8)work[0x0A] - workOffsetReferenceState;
          if (value < -0x30) {
            range = D_80146884;
            FIELD_REF(u8, range, 0x75) = 0xFD;
          } else if (value < -0x18) {
            range = D_80146884;
            FIELD_REF(u8, range, 0x75) = 0xFE;
          } else {
            range = D_80146884;
            FIELD_REF(u8, range, 0x75) = 0xFF;
          }
        }
      } else {
        work = D_1F800044;
        FIELD_REF(s32, work, 0x10) = 0;
        FIELD_REF(u8, range, 0x75) = 0;
      }

      work = D_1F800044;
      if (D_801E2AB8[work[6]].unk_20[0] != 0) {
        roll = rand();
        if ((roll & 1) != 0) {
          roll = rand();
          work = D_1F800044;
          if ((roll & 1) != 0) {
            FIELD_REF(s32, work, 0x0C) = 0x1000;
          } else {
            FIELD_REF(s32, work, 0x0C) = -0x1000;
          }
        } else {
          work = D_1F800044;
          FIELD_REF(s32, work, 0x0C) = 0;
        }
      } else {
        FIELD_REF(s32, work, 0x0C) = 0;
      }

      work = D_1F800044;
      work[8] = 1;
      work = D_1F800044;
      work[4] = 0;
    } else {
      roll = rand();
      if ((roll & 0x0F) < chanceRow[2]) {
        work = D_1F800044;
        FIELD_REF(s32, work, 0x0C) = 0;
        FIELD_REF(s32, work, 0x10) = 0;

        range = D_80146884;
        FIELD_REF(u8, range, 0x75) = 1;

        work = D_1F800044;
        work[4] = 1;
      } else {
        work = D_1F800044;
        if (work[0x0A] != 0) {
          range = D_80146884;
          FIELD_REF(u8, range, 0x75) = 0;

          work = D_1F800044;
          FIELD_REF(s32, work, 0x10) = -1;
          FIELD_REF(s32, work, 0x0C) = 0;
          work[4] = 2;
        }
      }
    }

    roll = rand();
    work = D_1F800044;
    work[9] = (u8)((8 - level) * ((roll & 3) + 1));
  } else {
    work[9] = (u8)(work[9] - 1);

    range = D_80146884;
    delta = FIELD_REF(s8, range, 0x75);
    if (delta > 0) {
      if ((D_80143E6C & 0x0F) == 0) {
        work = D_1F800044;
        if (FIELD_REF(s32, work, 0x10) >= 0) {
          limit = range->limit_8C;
          current = range->value_8E;
          if (current < (limit >> 1)) {
            func_801E026C((s8)delta);
          }
        }
      }
    } else if ((D_80143E6C & 1) != 0) {
      func_801E026C((s8)delta);
    }

    work = D_1F800044;
    value = (s32)(FIELD_REF(u32, work, 0x10) +
                  FIELD_REF(u32, work, 0x1C));
    work[0x0A] = (u8)(work[0x0A] + (u32)value);
  }

  work = D_1F800044;
  angle = (s8)work[0x0A];
  if (angle < -0x30) {
    angle = -0x30;
  } else if (angle > 0x30) {
    angle = 0x30;
  }
  work[0x0A] = (u8)angle;

  if (D_80149332 == 0) {
    D_8014932E = 0;
  }

  work = D_1F800044;
  reference = workOffsetReferenceState;
  angle = (s8)work[0x0A];
  if (angle > reference && FIELD_REF(s32, work, 0x10) > 0) {
    position = FIELD_REF(s32, work, 0x38);
    timer = FIELD_REF(s16, work, 0x3E);
    FIELD_REF(u32, work, 0x38) = (u32)position - 0x1000u;
    if (timer < 0) {
      FIELD_REF(s16, work, 0x3E) = (s16)(timer + 8);
    }

    work = D_1F800044;
    position = FIELD_REF(s32, work, 0x38);
    if (position > 0x150000) {
      D_8014932E = 0x10;
      D_8014930C = (u32)position;
      if (position > 0x3C0000) {
        D_8014930C = 0x3C0000;
      }
    }

    work = D_1F800044;
    velocity = FIELD_REF(s32, work, 0x0C);
    if (velocity < 0) {
      if (FIELD_REF(s32, work, 0x34) <= 0x60000) {
        FIELD_REF(u32, work, 0x0C) = 0u - (u32)velocity;
        work = D_1F800044;
      }
    } else {
      if (velocity > 0) {
        if (FIELD_REF(s32, work, 0x34) > 0x13FFFF) {
          FIELD_REF(u32, work, 0x0C) = 0u - (u32)velocity;
        }
      } else {
        FIELD_REF(s32, work, 0x0C) = 0;
      }
      work = D_1F800044;
    }

    velocity = FIELD_REF(s32, work, 0x0C);
    FIELD_REF(u32, work, 0x34) =
        FIELD_REF(u32, work, 0x34) + (u32)velocity;
    if (velocity < -0x800) {
      work[8] = 0;
    } else if (velocity > 0x800) {
      work[8] = 2;
    } else {
      work[8] = 1;
    }
  } else {
    work = D_1F800044;
    reference = workOffsetReferenceState;
    if ((s8)work[0x0A] < reference && D_8014421C < 0) {
      denominator = 0x3E - FIELD_REF(s16, work, 0x3A);
      numerator =
          (s32)(0xF0000u - FIELD_REF(u32, work, 0x34));
      numerator >>= 4;
      if (denominator != 0) {
        velocity = numerator / denominator;
        FIELD_REF(s32, work, 0x0C) = velocity;
        if (velocity < -0x1000) {
          velocity = -0x1000;
        } else if (velocity > 0x1000) {
          velocity = 0x1000;
        }
        FIELD_REF(s32, work, 0x0C) = velocity;
      } else {
        FIELD_REF(s32, work, 0x0C) = 0;
      }

      work = D_1F800044;
      position = FIELD_REF(s32, work, 0x34);
      velocity = FIELD_REF(s32, work, 0x0C);
      timer = FIELD_REF(s16, work, 0x3E);
      FIELD_REF(u32, work, 0x34) = (u32)position + (u32)velocity;
      if (timer < 0) {
        FIELD_REF(s16, work, 0x3E) = (s16)(timer + 8);
      }

      work = D_1F800044;
      position = (s32)(FIELD_REF(u32, work, 0x38) + 0x1000u);
      FIELD_REF(s32, work, 0x38) = position;
      if (position > 0x150000) {
        D_8014932E = 0x10;
        D_8014930C = (u32)position;
        if (position > 0x3C0000) {
          D_8014930C = 0x3C0000;
        }
      }

      work = D_1F800044;
      if (work[4] != 0) {
        work[8] = 5;
      } else {
        work[8] = 1;
      }
    }
  }

  work = D_1F800044;
  work[0x2A] = D_801E2330[work[8]].byte_01;

  work = D_1F800044;
  func_8014D8D4(D_801E2330[work[8]].request_00);
  func_8014D978();
}
