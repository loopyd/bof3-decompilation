#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8 *scenarioScena0800Work;

/* 0x801AFE18 — shared projection setup consuming a 0x20-byte stack scratch
 * area; only a0 is live at the call site. */
void func_801AFE18(void *arg0);

/* 0x801AFF04 — shared projector from a three-word point to a screen pair. */
void func_801AFF04(const void *arg0, void *arg1);

/* 0x801AFFD8 — shared size-pair converter: it converts the pair passed in arg1
 * for the point passed in arg0 and leaves the converted pair in arg2. */
void func_801AFFD8(const void *arg0, void *arg1, void *arg2);

/* 0x8017A138 — shared direction helper: it returns the 0x1000-unit angle of the
 * signed delta passed as (arg0, arg1). */
s32 func_8017A138(s32 arg0, s32 arg1);

/* 0x801F9420 — this overlay's two-value average on the 0x1000-unit rotation
 * scale. */
s32 average12BitValuesScenarioScena0800_801F9420(u32 arg0, u32 arg1);

/* One 0x18-byte trail record: the projected point words at 0x00-0x08, the
 * projected screen pair at 0x10/0x12, the averaged heading halfword at 0x14 and
 * the converted size halfword at 0x16. The whole record is shifted as one
 * 24-byte unit, so only the fields the walk touches are modelled here. */
typedef struct ScenarioScena0800TrailRecord {
  s32 x_00;
  s32 y_04;
  s32 z_08;
  s32 field_0C;
  u16 screenX_10;
  u16 screenY_12;
  u16 heading_14;
  u16 size_16;
} ScenarioScena0800TrailRecord;

/* The 0x340-byte trail buffer: 0x20 records of 0x18-byte stride followed by the
 * 0x20 segment-heading halfwords at 0x300. Entry 0x1F of that halfword run is
 * the shared halfword at 0x800E4C1E. */
typedef struct ScenarioScena0800TrailBuffer {
  ScenarioScena0800TrailRecord records[0x20];
  u16 headings[0x20];
} ScenarioScena0800TrailBuffer;

/* @source 0x801F8D98
 * @behavior Rebuilds the overlay trail: it first shifts the 0x20 records of the
 * 0x18-byte-stride buffer up by one so the newest position lands in record 0,
 * which it seeds from the cursor words 0x34/0x38/0x3C of the scratchpad work
 * object. It then fills the 0x20-byte projection scratch, walks all 0x20
 * records projecting each one through the shared projector, and converts the
 * pair taken from the last heading halfword into each record's size halfword.
 * A second walk takes the signed screen delta between each record and its
 * successor and stores the shared direction helper's result as that segment's
 * heading, writing the 0x1000 sentinel for a degenerate delta. A third walk
 * fills every sentinel heading with the next valid heading, or, when nothing
 * ahead is valid, with the first valid heading behind it, or 0 when the run
 * holds none. Finally it averages each heading with its predecessor through the
 * overlay average helper into the record heading halfwords, seeding record 0
 * with the first heading and record 0x1F with the last one. Takes the buffer
 * pointer in a0 and returns nothing.
 * @status partial
 * @match 97.55
 * @residual Five heading-address additions in the sentinel-fill walk have
 *   their operands reversed. All other instructions, the 816-byte size and
 *   the 0x58 frame match.
 */
void advanceTrailProjectPointsFillHeadingsAndAverageAnglesScenarioScena0800_801F8D98(
    u8 *arg0) {
  ScenarioScena0800TrailBuffer *buffer;
  u8 *work;
  s8 i;
  s8 j;
  s32 prev;
  u16 sxy[4];
  s16 dxy[2];
  u8 scratch[0x20];
  u16 pair;
  s32 off;
  s32 off2;

  buffer = (ScenarioScena0800TrailBuffer *)arg0;

  for (i = 0x1E; i >= 0; i--) {
    buffer->records[i + 1] = buffer->records[i];
  }

  work = scenarioScena0800Work;
  buffer->records[0].x_00 = *(s32 *)(work + 0x34);
  buffer->records[0].y_04 = *(s32 *)(work + 0x38);
  buffer->records[0].z_08 = *(s32 *)(work + 0x3C);
  func_801AFE18(scratch);

  for (i = 0; i < 0x20; i++) {
    func_801AFF04(&buffer->records[i], &buffer->records[i].screenX_10);
    pair = buffer->headings[0x1F];
    sxy[1] = pair;
    sxy[0] = pair;
    func_801AFFD8(&buffer->records[i], sxy, dxy);
    buffer->records[i].size_16 = *(u16 *)dxy;
  }

  /* The delta and average walks reach the heading slot through the byte offset
   * in `off`/`off2` rather than the `headings` member: the member subscript
   * makes this compiler build the address index-first, while the original's
   * words carry the buffer operand first. 0x300 is the start of the heading run
   * in bytes, halfword 0x180. */
  for (i = 0; i < 0x1F; i++) {
    dxy[0] = buffer->records[i].screenX_10 - buffer->records[i + 1].screenX_10;
    dxy[1] = buffer->records[i].screenY_12 - buffer->records[i + 1].screenY_12;
    off = i * 2;
    if (dxy[0] != 0 || dxy[1] != 0) {
      *(u16 *)((u8 *)buffer + off + 0x300) = func_8017A138(dxy[1], dxy[0]);
    } else {
      *(u16 *)((u8 *)buffer + off + 0x300) = 0x1000;
    }
  }

  for (i = 0; i < 0x1F; i++) {
    if (buffer->headings[i] != 0x1000) {
      continue;
    }
    /* Both searches share one index: the forward value is dead before the
     * backward search initializes it, including the all-sentinel path. */
    for (j = i + 1; j < 0x1F; j++) {
      if (buffer->headings[j] != 0x1000) {
        buffer->headings[i] = buffer->headings[j];
        goto headingFilled;
      }
    }
    for (j = 0; j < i; j++) {
      if (buffer->headings[j] != 0x1000) {
        buffer->headings[i] = buffer->headings[j];
        goto headingFilled;
      }
    }
    buffer->headings[j] = 0;
  headingFilled:;
  }

  buffer->records[0].heading_14 = buffer->headings[0];
  for (i = 1; i < 0x1F; i++) {
    prev = (i - 1) * 2;
    off2 = i * 2;
    buffer->records[i].heading_14 =
        average12BitValuesScenarioScena0800_801F9420(
            *(u16 *)((u8 *)buffer + prev + 0x300),
            *(u16 *)((u8 *)buffer + off2 + 0x300));
  }
  buffer->records[0x1F].heading_14 = buffer->headings[0x1E];
}
