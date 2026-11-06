#include "bof3/world/area03004_internal.h"

/* Per-evaluation reload of the scratch work-record cursor cell, matching the
 * original's repeated lui/lw pairs before each access group. Each reload group
 * keeps its own pointer local: one reused cursor became a single global pseudo
 * whose register conflicted with the two the original reads into (v1 for the
 * mode-0 bounce and v0 for the gradient-bar tail). */
#define AREA030_SCRATCH ((u8*)D_1F800044)

/* @source 0x801D1CF0
 * @behavior AREA030 message-panel fade step, handler index 2 of the work-byte-1
 * table 0x801E1D10. While the work velocity word 0x18 is non-zero it advances
 * the panel fade level in work word 0x0C under byte 1 of the active entry
 * record D_801E320C: mode 0 bounces the level against 0x00/0xFF and reverses
 * the velocity, mode 1 wraps the level back to 0x00 once it reaches 0xFF, and
 * mode 2 moves the second accumulator (work word 0x10) one step off its -1
 * floor and adds it to the fade level, clamping and reversing at the same
 * 0x00/0xFF limits. It then selects tpage draw mode 0/2, draws the message
 * panel box func_801E0DCC(0x10, 2, 0x87, 0xB4) and the green gradient bar at
 * (0x88, 0xCB) whose width is the fade level scaled to the 0x60 ramp maximum.
 * Once the shared phase byte 0x80145E92 reads 3 it arms the box halfwords
 * 0x2E/0x30 with 0xB8/0xD1 and advances the work state byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advancePanelFadeAndDrawGradientBarWorld00Area03004_801D1CF0(void) {
  u8* work;
  u8* bounce;
  u8* clamp;
  u8* bar;
  u8* box;
  s32 step;
  s32 level;
  s16 width;

  work = AREA030_SCRATCH;
  step = *(s32*)(work + 0x18);
  if (step != 0) {
    switch (((u8*)D_801E320C)[1]) {
    case 0:
      level = *(s32*)(work + 0x0C) + step;
      *(s32*)(work + 0x0C) = level;
      if (*(s32*)(work + 0x18) > 0 && level >= 0xFF) {
        *(s32*)(work + 0x0C) = 0xFF;
        *(s32*)(work + 0x18) = -*(s32*)(work + 0x18);
      } else {
        bounce = AREA030_SCRATCH;
        if (*(s32*)(bounce + 0x18) < 0 && *(s32*)(bounce + 0x0C) <= 0) {
          *(s32*)(bounce + 0x0C) = 0;
          *(s32*)(bounce + 0x18) = -*(s32*)(bounce + 0x18);
        }
      }
      break;
    case 1:
      if (*(s32*)(work + 0x0C) == 0xFF) {
        *(s32*)(work + 0x0C) = 0;
      } else {
        *(s32*)(work + 0x0C) = *(s32*)(work + 0x0C) + step;
        if (*(s32*)(work + 0x0C) >= 0x100) {
          *(s32*)(work + 0x0C) = 0xFF;
        }
      }
      break;
    case 2:
      level = *(s32*)(work + 0x10);
      if (level < 0) {
        *(s32*)(work + 0x10) = level + step;
        if (*(s32*)(work + 0x10) >= 0) {
          *(s32*)(work + 0x10) = -1;
        }
      } else {
        *(s32*)(work + 0x10) = level + step;
      }
      clamp = AREA030_SCRATCH;
      level = *(s32*)(clamp + 0x0C) + *(s32*)(clamp + 0x10);
      *(s32*)(clamp + 0x0C) = level;
      if (*(s32*)(clamp + 0x10) > 0) {
        if (level >= 0xFF) {
          *(s32*)(clamp + 0x0C) = 0xFF;
          *(s32*)(clamp + 0x10) = -*(s32*)(clamp + 0x10);
        }
      } else if (level <= 0) {
        *(s32*)(clamp + 0x0C) = 0;
        *(s32*)(clamp + 0x10) = 0;
      }
      break;
    }
  }

  submitTpageDrawMode(0, 2);
  func_801E0DCC(0x10, 2, 0x87, 0xB4);
  bar = AREA030_SCRATCH;
  width = (s16)(*(s32*)(bar + 0x0C) * 0x60 / 0xFF);
  drawGreenGradientBar(0x88, 0xCB, width, 2);

  if (D_80145E92 == 3) {
    /* The single-execution scope keeps the real store value ahead of the
     * cursor reload in GCC 2.7.2, allowing branch reorg to fill its delay slot.
     * The consumed width local also holds the later byte value (0..255). */
    do {
      width = 0xB8;
    } while (0);
    box = AREA030_SCRATCH;
    *(u16*)(box + 0x2E) = width;
    *(u16*)(box + 0x30) = 0xD1;
    width = box[1];
    box[1] = width + 1;
  }
}
