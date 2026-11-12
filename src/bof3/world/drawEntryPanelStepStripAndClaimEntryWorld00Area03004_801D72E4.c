#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 entry-panel drawer for the selected step, painted at the
 * requested origin: it shades the 0xF2 by 0x9F panel at (x, y) through
 * func_801AE3F0 with shade 0xF0 and the CLUT-bank byte D_80144952, then, while
 * work byte 8 (the slide direction) is nonzero, drives the step strip. When
 * work byte 9 is clear it re-targets the strip by storing nine frames per step
 * (word 0x20 takes the selected step word 0x3C times 9 and the displayed step
 * word 0x3E takes 0x3C) whenever the two disagree, steps the strip clock word
 * 0x14 down with a 0x35 wrap while the direction is positive, advances work
 * byte 9 and then byte 0xA. Phase byte 0xA zero is the settle branch: when the
 * strip clock has reached its target word 0x20 it clears bytes 8 and 9 and
 * claims a front-end entry record through func_8019601C, seeding that 0x74-byte
 * row with flags 1, handler selector 0x1A at +5, byte 1 = 0x10 and byte 2 = the
 * low byte of the displayed step word 0x3E; otherwise it steps the strip clock
 * down the same way and advances the phase. A nonzero phase only advances, and
 * the advance to 4 restarts it at zero while stepping the strip clock up with a
 * 0x36 wrap in the negative direction. It then draws the animated strip: nine
 * sprites from y + 0x19 at phase zero, otherwise ten sprites from
 * y + 3*phase + 0xC in the positive direction or y - (3*phase - 0x19) in the
 * negative one, one per 0xD row, skipping the 0xFFFF sentinel and walking the
 * 54-entry halfword sprite table D_801E2120 from the strip clock with
 * wraparound, each sprite resolved through the main-executable VRAM offset
 * table at the 0x80010000 base. Finally it paints the 0xF2 by 0xE header bar at
 * y + 6 and the 0xF2 by 0xB footer bar at y + 0x94 (shade 0x30), draws the
 * selected step icon - sprite D_801E2114[step word 0x3C] - at (x + 0x3D,
 * y + 6), draws the status icon row through func_801D7784, and formats the
 * selected step word 0x3C plus one through the "%d/6" rodata string D_801D0C08
 * into the shared 0x80145AD4 buffer, submitting it at (x + 0xCA, y + 0x92)
 * through func_80150098.
 * @source 0x801D72E4
 * @status partial
 * @match 96.96
 * @residual the strip-start pseudo is colored v0 where the original uses a1,
 * which keeps its phase-zero store out of the beqz delay slot, moves the loop
 * preheader copy of that value ahead of the strip clock read, and leaves the
 * footer CLUT address unfolded into each load instead of hoisted once; body
 * size, frame and every instruction shape otherwise agree.
 */
/* The published cursor cell 0x1F800044 is volatile, so every access group
 * re-reads it and keeps its own pointer local, as the record-family siblings
 * func_801D6CB0 and decrementEntryPanelSlideInitializeRecordAndAdvanceStateWorld00Area03004_801D5398
 * do. Three groups are separate variables because the original keeps them in
 * three registers: the entry read, the body that steps the strip clock, and the
 * strip and footer read. The settle read of the displayed step word is its own
 * local, exactly as the donor decrementEntryPanelSlideInitializeRecordAndAdvanceStateWorld00Area03004_801D5398
 * spells it, and that donor also supplies the entry-record seeding statement
 * order, the (u8) allocator result and the (u16) field reference. The
 * VRAM-offset indexing spelling comes from the exact func_801E162C. This span's
 * own literals - the 0xF2/0x9F panel, the 0x35 and 0x36 clock wraps, the nine
 * frames per step, the 0xC/0x19/0xD strip geometry, the 0x3D/0xCA/0x92 footer
 * offsets and the "%d/6" format - come from its original bytes.
 *
 * Measured spellings on the way to this body, each at the same size: one cursor
 * variable for every reload group scored 231/296 instructions (78.04%), because
 * one shared pseudo took one register for the whole function; splitting the
 * cursor per reload group scored 253/296, and splitting the strip-clock
 * temporary per site (tick, limit, stepTick, span) scored 277/296. The
 * pre-loop copy into the row cursor took it to 287/296 at the payload's own
 * 1184 bytes. A single row-cursor variable with no copy scored 277/296, and
 * inline or pre-branch geometry arithmetic scored 286 and 271.
 */
void drawEntryPanelStepStripAndClaimEntryWorld00Area03004_801D72E4(s32 x,
                                                                   s32 y) {
  u8* work;
  u8* clock;
  u8* phase;
  u8* step;
  u8* restart;
  u8* values;
  u8* panel;
  u8* strip;
  s32 startY;
  s32 top;
  s32 count;
  s32 row;
  s32 tick;
  s32 limit;
  s32 stepTick;
  s32 span;
  u8 frame;
  u16 sprite;
  u32 index;

  func_801ae3f0((u16)x, (u16)y, 0xF2, 0x9F, 0xF0, D_80144952);

  work = D_1F800044;
  if (work[8] != 0) {
    if (work[9] == 0) {
      if (FIELD_REF(s16, work, 0x3E) != FIELD_REF(u16, work, 0x3C)) {
        FIELD_REF(s32, work, 0x20) = FIELD_REF(u16, work, 0x3C) * 9;
        FIELD_REF(u16, work, 0x3E) = FIELD_REF(u16, work, 0x3C);
      }
      clock = D_1F800044;
      if ((s8)clock[8] > 0) {
        tick = FIELD_REF(s32, clock, 0x14);
        tick = tick - 1;
        FIELD_REF(s32, clock, 0x14) = tick;
        if (tick < 0) {
          FIELD_REF(s32, clock, 0x14) = 0x35;
        }
        clock = D_1F800044;
      }
      clock[9]++;
      step = D_1F800044;
      step[10]++;
    }
    phase = D_1F800044;
    if (phase[10] == 0) {
      limit = FIELD_REF(s32, phase, 0x14);
      if (limit == FIELD_REF(s32, phase, 0x20)) {
        phase[9] = 0;
        D_1F800044[8] = 0;
        index = (u8)func_8019601c();
        D_80143FC8[index].flags_00 = 1;
        D_80143FC8[index].byte_05 = 0x1A;
        D_80143FC8[index].unk_01 = 0x10;
        values = D_1F800044;
        D_80143FC8[index].unk_02 = FIELD_REF(u16, values, 0x3E);
      } else {
        if ((s8)phase[8] > 0) {
          FIELD_REF(s32, phase, 0x14) = limit - 1;
          if (limit - 1 < 0) {
            FIELD_REF(s32, phase, 0x14) = 0x35;
          }
        }
        phase = D_1F800044;
        phase[10]++;
      }
    } else {
      phase[10]++;
      restart = D_1F800044;
      if (restart[10] == 4) {
        restart[10] = 0;
        step = D_1F800044;
        if ((s8)step[8] < 0) {
          stepTick = FIELD_REF(s32, step, 0x14);
          stepTick = stepTick + 1;
          FIELD_REF(s32, step, 0x14) = stepTick;
          if (stepTick >= 0x36) {
            FIELD_REF(s32, step, 0x14) = 0;
          }
        }
      }
    }
  }

  panel = D_1F800044;
  if (panel[10] != 0) {
    if (panel[8] == 1) {
      span = panel[10] * 3 + 0xC;
      top = y + span;
    } else {
      span = panel[10] * 3 - 0x19;
      top = y - span;
    }
    count = 10;
  } else {
    top = y + 0x19;
    count = 9;
  }
  startY = top;

  strip = D_1F800044;
  frame = strip[0x14];
  for (row = 0; row < count; row++) {
    sprite = D_801E2120[frame];
    if (sprite != 0xFFFF) {
      func_8014f800((s16)(x + 6), (s16)startY, 0, 0xFF,
                    0x80010000u + PSX_PTR(u16, 0x80010000u)[sprite]);
    }
    frame++;
    if (frame >= 0x36) {
      frame = 0;
    }
    startY += 0xD;
  }

  func_801ae3f0((u16)x, (u16)(y + 6), 0xF2, 0xE, 0, D_80144952);
  func_801ae3f0((u16)x, (u16)(y + 0x94), 0xF2, 0xB, 0x30, D_80144952);
  func_8014f800((s16)(x + 0x3D), (s16)(y + 6), 0, 0xFF,
                0x80010000u +
                    PSX_PTR(u16, 0x80010000u)[D_801E2114[FIELD_REF(
                        u16, D_1F800044, 0x3C)]]);
  func_801D7784(x, y);
  sprintf((char*)D_80145AD4, (const char*)D_801D0C08,
          FIELD_REF(u16, D_1F800044, 0x3C) + 1);
  func_80150098((s16)(x + 0xCA), (s16)(y + 0x92), 0, D_80145AD4);
}
