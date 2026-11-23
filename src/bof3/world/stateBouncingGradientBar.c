#include "bof3/world/area03004_internal.h"

/* Three-byte AREA030 bounce-step records at 0x801E2008, the same record layout
 * as the sibling panel-step table 0x801E2024: `resource` supplies the active-work sequence selector passed to selectActiveWorkSequence and carries the 0xFF end marker of the run,
 * `countdown` is reloaded into work byte 9 and doubles as the restart index when
 * the record is the end marker, and `mode` selects the work byte 6 behaviour -
 * 0 integrates the bar-width delta in work word 0x18 into the width in work word
 * 0x0C, 1 advances work byte 6 on every other frame, and the end record's 0 does
 * neither. All three fields are read with an `lbu`, so the end marker compares
 * against 0xFF as an unsigned byte. */
typedef struct Area030BounceStepRecord {
  u8 resource;
  u8 countdown;
  u8 mode;
} Area030BounceStepRecord;

extern Area030BounceStepRecord bounceStepRecordTable[]; /* @source 0x801E2008 @kind table */

/* The AREA030 scratch work record viewed at the three offsets this handler
 * re-seeds, proven by its own stores: the work byte 6 readout counter, the work
 * word 0x0C the gradient bar takes as its width, and the work word 0x18 that
 * carries the width delta the sibling initializer initializeGradientBarPanel seeds with
 * 4. */
typedef struct Area030BounceStepState {
  u8 pad_00[0x06];
  u8 count_06;
  u8 pad_07[5];
  s32 width_0C;
  u8 pad_10[8];
  s32 delta_18;
} Area030BounceStepState;

/* @source 0x801D5D88
 * @behavior AREA030 panel-step handler, index 1 of the work-byte-3 dispatch table
 * 0x801E2000: while work byte 9 of the record published at the scratchpad
 * cursor 0x1F800044 is nonzero it decrements that byte and runs the shared
 * func_8014D978 helper; otherwise it reads work byte 0xA as an index into the
 * three-byte record table 0x801E2008 and, when the record carries the 0xFF end
 * marker, re-seeds work byte 0xA from the record countdown byte and restarts
 * the bar animation the sibling initializer
 * initializeGradientBarPanel set up - width
 * 0 in work word 0x0C, delta 4 in work word 0x18 and work byte 6 cleared. It
 * then selects the sequence identified by the record through
 * selectActiveWorkSequence, reloads work byte 9 from the record countdown and
 * advances work byte 0xA. Every call then runs queueUnprojectedActiveWork,
 * submits texture-page draw mode 0/1 and appends the sprites (0x15, 1, 0x32,
 * 0x7A), (0x16, 1, 0xBC, 0x7A) and (0x10, 1, 0x36, 0x80) through
 * appendIndexedSprite. The stepped-from record's mode byte
 * then drives the animation: mode 0 moves the width by the sign-carrying delta
 * and reflects it at 0 and 0x60 (the delta is negated at either end, so the
 * bar bounces), while mode 1 advances work byte 6 once every other frame.
 * Finally it draws the gradient bar of that width at (0x39, 0x97) in append
 * mode 1, formats work byte 6 through the "%2d" rodata string 0x801D0C04 into
 * the shared 0x80145AD4 buffer and submits the buffer at (0xAC, 0x94) in mode
 * 0 through func_8014FF0C.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published cursor cell 0x1F800044 is volatile, so every access group
 * re-reads it and keeps its own pointer local, as the exact siblings
 * advancePanelStepSequenceAndDrawBouncingMarkerWorld00Area03004_801D6128 and
 * stateEntryCountValue do: the
 * entry read, the end-marker re-seed, the post-call resource read, the countdown
 * reload, the index advance, the stepped-from mode read, the mode 0 width
 * integration, the bar-width read-back and the readout formatting read. The
 * statement order, the (u8*) local-per-group style and the
 * `sprintf((char*)buf, (const char*)D_801D0C04, ...)` / `func_8014FF0C` tail come
 * from those siblings; this span's own table, priorities, END marker 0xFF, width
 * delta 4, bounce bounds 0/0x60, frame-counter bit 1 and draw coordinates come
 * from the original bytes.
 */
void stateBouncingGradientBar(void) {
  u8* work;
  u8* cursor;
  u8* reload;
  u8* next;
  u8* stepped;
  u8* buf;
  s32 mode;
  s32 width;
  u8 step;
  Area030BounceStepState* state;

  work = D_1F800044;
  if (work[9] == 0) {
    step = work[0xA];
    if (bounceStepRecordTable[step].resource == 0xFF) {
      s32 delta;
      do {
        delta = 4;
      } while (0);
      work[0xA] = bounceStepRecordTable[step].countdown;
      state = (Area030BounceStepState*)D_1F800044;
      state->width_0C = 0;
      state->delta_18 = delta;
      state->count_06 = 0;
    }
    cursor = D_1F800044;
    selectActiveWorkSequence(
        bounceStepRecordTable[cursor[0xA]].resource);
    reload = D_1F800044;
    reload[9] =
        bounceStepRecordTable[reload[0xA]].countdown;
    next = D_1F800044;
    next[0xA] = next[0xA] + 1;
  } else {
    work[9] = work[9] - 1;
    func_8014D978();
  }

  queueUnprojectedActiveWork();
  submitTpageDrawMode(0, 1);
  appendIndexedSprite(0x15, 1, 0x32, 0x7A);
  appendIndexedSprite(0x16, 1, 0xBC, 0x7A);
  appendIndexedSprite(0x10, 1, 0x36, 0x80);

  stepped = D_1F800044;
  mode = bounceStepRecordTable[stepped[0xA] - 1].mode;
  if (mode == 0) {
    width = *(s32*)(stepped + 0x0C) + *(s32*)(stepped + 0x18);
    *(s32*)(stepped + 0x0C) = width;
    if (*(s32*)(stepped + 0x18) > 0) {
      if (width >= 0x61) {
        *(s32*)(stepped + 0x0C) = 0x60;
        *(s32*)(stepped + 0x18) = -*(s32*)(stepped + 0x18);
      }
    } else if (width < 0) {
      *(s32*)(stepped + 0x0C) = 0;
      *(s32*)(stepped + 0x18) = -*(s32*)(stepped + 0x18);
    }
  } else if (mode == 1 && (D_80143E6C & 1) == 0) {
    stepped[6] = stepped[6] + 1;
  }

  drawGreenGradientBar(0x39, 0x97, *(s16*)(D_1F800044 + 0x0C), 1);
  buf = D_80145AD4;
  sprintf((char*)buf, (const char*)D_801D0C04, D_1F800044[6]);
  func_8014FF0C(0xAC, 0x94, 0, buf);
  queueUnprojectedActiveWork();
}
