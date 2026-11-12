#include "bof3/bof3.h"

/* Live work object published by the scratchpad pointer cell; only the cursor
 * words this body re-reads from it are used.
 * @source 0x1F800044 @kind unknown
 */
extern u8 *D_1F800044;

/* Shared projection setup consuming a 0x20-byte stack scratch area; only a0 is
 * live at the call site.
 * @source 0x801AFE18
 */
extern void func_801AFE18(void *arg0);

/* Shared projector from a three-word point to a screen pair.
 * @source 0x801AFF04
 */
extern void func_801AFF04(const void *arg0, void *arg1);

/* Shared size-pair converter: it converts the pair passed in arg1 for the point
 * passed in arg0 and leaves the converted pair in arg2.
 * @source 0x801AFFD8
 */
extern void func_801AFFD8(const void *arg0, void *arg1, void *arg2);

/* Shared direction helper: it returns the 0x1000-unit angle of the signed delta
 * passed as (arg0, arg1).
 * @source 0x8017A138
 */
extern s32 func_8017A138(s32 arg0, s32 arg1);

/* This overlay's two-value average on the 0x1000-unit rotation scale. */
s32 average12BitValuesWorld02Area08813_801F35B0(u32 arg0, u32 arg1);

/* One 0x18-byte trail record: the projected point words at 0x00-0x08, the
 * projected screen pair at 0x10/0x12, the averaged heading halfword at 0x14 and
 * the converted size halfword at 0x16. The whole record is shifted as one
 * 24-byte unit, so only the fields the walk touches are modelled here. */
typedef struct World02Area08813TrailRecord {
  s32 x_00;
  s32 y_04;
  s32 z_08;
  s32 field_0C;
  u16 screenX_10;
  u16 screenY_12;
  u16 heading_14;
  u16 size_16;
} World02Area08813TrailRecord;

/* The 0x340-byte trail buffer: 0x20 records of 0x18-byte stride followed by the
 * 0x20 segment-heading halfwords at 0x300. Entry 0x1F of that halfword run is
 * the size seed the caller stores before this body runs. */
typedef struct World02Area08813TrailBuffer {
  World02Area08813TrailRecord records[0x20];
  u16 headings[0x20];
} World02Area08813TrailBuffer;

/* @source 0x801F2F28
 * @behavior Rebuilds the trail buffer passed in a0. It first shifts the 0x20
 * records of the 0x18-byte-stride buffer up by one so the newest position lands
 * in record 0, which it seeds from the cursor words 0x34/0x38/0x3C of the work
 * object published at the scratchpad pointer cell 0x1F800044. It then fills the
 * 0x20-byte projection scratch, walks all 0x20 records projecting each one
 * through the shared 0x801AFF04 projector into its screen pair at +0x10/+0x12,
 * and hands that record with a size pair copied twice from heading halfword
 * 0x1F to the shared 0x801AFFD8 converter, whose first result halfword becomes
 * the record size at +0x16. A second walk takes the signed screen delta between
 * each record and its successor and stores the shared 0x8017A138 direction
 * helper result of that delta as the segment heading in halfword run 0x300,
 * writing the 0x1000 sentinel for a degenerate delta. A third walk fills every
 * sentinel heading with the next valid heading, or, when nothing ahead is
 * valid, with the first valid heading behind it, or 0 when the run holds none.
 * Finally it averages each heading with its predecessor through this overlay's
 * 12-bit average at 0x801F35B0 into record halfword 0x14, seeding record 0 with
 * heading 0 and record 0x1F with heading 0x1E. It keeps a 0x58-byte frame for
 * $ra, $s0-$s3, the 0x20-byte projection scratch, the 8-byte pair handed to the
 * converter and the 4-byte delta pair, and returns nothing.
 *
 * The 0x330-byte span is the same body as the exact-twin spans at 0x801F8D98 of
 * emi/scenario/scena08/00 and 0x801F88B8 of emi/scenario/scena07/00: a word
 * comparison of the three payloads differs in five absolute j/jal target words
 * only, each shifted by exactly the distance between the spans (0x5E70 and
 * 0x5990). All four in-image call sites of this address pass the fixed buffer
 * 0x800E4800 in $a0, and one of them stores 0x60 to 0x800E4B3E - heading
 * halfword 0x1F of that buffer - before calling, which is the size seed this
 * body copies into each record. The scenario08 twin was lifted as a partial
 * whose residual was the heading-address operand order, so this body carries
 * the sentinel fill's index as an explicit shift instead of a multiply, which
 * was the spelling that measured the buffer-first operand order of that walk;
 * the other two walks keep the byte offset in a local, which measured it too.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceTrailProjectPointsFillHeadingsAndAverageAnglesWorld02Area08813_801F2F28(
    u8 *arg0) {
  World02Area08813TrailBuffer *buffer;
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

  buffer = (World02Area08813TrailBuffer *)arg0;

  for (i = 0x1E; i >= 0; i--) {
    buffer->records[i + 1] = buffer->records[i];
  }

  work = D_1F800044;
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
   * builds the address index-first for this compiler build, as the scenario08
   * twin of this body measured, while the original's words carry the buffer
   * operand first. 0x300 is the start of the heading run in bytes, halfword
   * 0x180. */
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

  /* The sentinel fill reaches the same run the same way, and its index is an
   * explicit shift rather than a multiply: the multiply spelling made this
   * compiler emit every heading address as `addu index, buffer` where the
   * original carries `addu buffer, index` at all five of these sites. */
  for (i = 0; i < 0x1F; i++) {
    if (*(u16 *)((u8 *)buffer + (i << 1) + 0x300) != 0x1000) {
      continue;
    }
    /* Both searches share one index: the forward value is dead before the
     * backward search initializes it, including the all-sentinel path. */
    for (j = i + 1; j < 0x1F; j++) {
      if (*(u16 *)((u8 *)buffer + (j << 1) + 0x300) != 0x1000) {
        *(u16 *)((u8 *)buffer + (i << 1) + 0x300) =
            *(u16 *)((u8 *)buffer + (j << 1) + 0x300);
        goto headingFilled;
      }
    }
    for (j = 0; j < i; j++) {
      if (*(u16 *)((u8 *)buffer + (j << 1) + 0x300) != 0x1000) {
        *(u16 *)((u8 *)buffer + (i << 1) + 0x300) =
            *(u16 *)((u8 *)buffer + (j << 1) + 0x300);
        goto headingFilled;
      }
    }
    *(u16 *)((u8 *)buffer + (j << 1) + 0x300) = 0;
  headingFilled:;
  }

  buffer->records[0].heading_14 = buffer->headings[0];
  for (i = 1; i < 0x1F; i++) {
    prev = (i - 1) * 2;
    off2 = i * 2;
    buffer->records[i].heading_14 =
        average12BitValuesWorld02Area08813_801F35B0(
            *(u16 *)((u8 *)buffer + prev + 0x300),
            *(u16 *)((u8 *)buffer + off2 + 0x300));
  }
  buffer->records[0x1F].heading_14 = buffer->headings[0x1E];
}
