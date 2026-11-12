#include "bof3/bof3.h"

/* Panel task root this overlay entry draws. Its prefix repeats the shared panel
 * layout (the two origin halfwords at +0x04 and +0x06); the six bytes the entry
 * walk reads follow it, and the two halfwords that close the run carry the
 * stream selector and the row window. Only the offsets this body touches are
 * named, and the roles no byte proves stay address anchored.
 */
typedef struct ShisuEquipCategoryAvailabilityTask {
  u8 unk_00[4];
  u16 x;         /* +0x04 panel origin column */
  u16 y;         /* +0x06 panel origin row */
  u8 unk_08;     /* +0x08 selector handed to the availability test */
  u8 unk_09;     /* +0x09 shade selector byte of the three inset paints */
  u8 unk_0A;     /* +0x0A equipment category, indexing both entry lists */
  u8 unk_0B;     /* +0x0B selected entry index and row-window first index */
  u8 unk_0C;     /* +0x0C row index emitted as a highlighted row */
  u8 unk_0D;     /* +0x0D row index emitted as a highlighted row */
  u8 unk_0E[2];  /* +0x0E */
  u16 unk_10;    /* +0x10 stream selector halfword, stepped down by 0x10 */
  u16 unk_12;    /* +0x12 row-window halfword the row-window query reads */
} ShisuEquipCategoryAvailabilityTask;

/* Shared CLUT-bank byte of the main executable that every SHISU panel passes as
 * the trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared work byte of the main executable: this body sign-extends it as the
 * index of the two mapping tables below.
 * @source 0x80148656 @kind data */
extern u8 D_80148656;

/* The two mapping tables of the main executable the availability test chains to
 * reach a character record: D_80181B10[D_80144F5A[character]] is that record
 * index.
 * @source 0x80144F5A @kind table */
extern const u8 D_80144F5A[];
/* @source 0x80181B10 @kind table */
extern const u8 D_80181B10[];

/* The two parallel equipment lists of the main executable, selected by the
 * category byte: D_801C893C carries the entry ids the row walk emits and
 * D_801C8950 the byte drawn beside each id. Both are work RAM of the main
 * executable and hold no initialized word in the image.
 * @source 0x801C893C @kind table */
extern u8 *D_801C893C[];
/* @source 0x801C8950 @kind table */
extern u8 *D_801C8950[];

/* Shared sprintf work buffer of the main executable: the category counter is
 * formatted here and then handed to the line-text drawer.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* The counter format of this overlay's leading data blob at payload byte 0x140:
 * the run "%3d/%3d", filled with the category's entry count and its capacity.
 * @source 0x801D0D40 @kind data */
extern const char D_801D0D40[];

/* Caption table of this overlay's trailing blob, indexed by the category byte:
 * its five entries point at the runs ITEM, WEAPON, ARMOR, OPTION and VITAL that
 * sit just ahead of it at payload bytes 0x9248 through 0x9268. The selected run
 * is measured with func_801AF20C and drawn right-aligned inside the panel.
 * @source 0x801D9E70 @kind table */
extern const u8 *D_801D9E70[];

/* The four 0xC-byte sprite-record streams of the same blob, each submitted at
 * the panel origin with the flag 1: the first pair is selected by bit 0x02 of
 * the task halfword at +0x10 and the second pair by bit 0x01.
 * @source 0x801D9960 @kind data */
extern const u8 D_801D9960[];
/* @source 0x801D9978 @kind data */
extern const u8 D_801D9978[];
/* @source 0x801D996C @kind data */
extern const u8 D_801D996C[];
/* @source 0x801D9984 @kind data */
extern const u8 D_801D9984[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Row-window query of the main executable: the selected-entry byte, the two
 * work bytes it fills and the task halfword it reads, answered with the offset
 * of the first visible entry inside the equipment list.
 * @source 0x801AFD10 */
extern u32 func_801AFD10(u8 *arg0, u8 *arg1, u8 *arg2, u16 *arg3);

/* Availability test of the main executable: the task's selector byte, the
 * character record index, the category byte and the entry id, answered through
 * its low byte. A zero answer marks the row unavailable.
 * @source 0x801AF494 */
extern u32 func_801AF494(s32 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Entry-row emitter of the main executable: the two signed coordinates, the
 * shade, the category byte, the entry id, the byte drawn beside it and the row
 * flag. Only the positions this call site fills are evidenced, so every role
 * stays positional.
 * @source 0x801AF800 */
extern void func_801AF800(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4,
                          u8 arg5, s32 arg6);

/* String measure of the main executable: it answers the length of the caption
 * run, which this body reads through its low byte and scales by four cells.
 * @source 0x801AF20C */
extern u32 func_801AF20C(const void *arg0);

/* Glyph-run submitter of the main executable: the two signed coordinates, the
 * shade, the glyph width and the address of the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Category entry-count lookup of the main executable: the category byte answers
 * the number of entries the equipment list of that category holds, which this
 * body formats as the counter's first field.
 * @source 0x80166438 */
extern u8 func_80166438(u32 index);

/* PsyQ libc formatter, called for its store into the shared buffer. */
extern int sprintf(char *buffer, const char *format, ...);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * shade argument, the alignment argument and the string buffer it draws.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8 *arg3);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);

/* Sprite emitter of the main executable: it fills one GT quad from the
 * sprite-record table entry the id selects and appends it at slot 1.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* Icon-grid emitter of the main executable: the entry list, the first-index
 * byte, the two signed coordinates and the three grid parameters 9, 0x20 or
 * 0x80 and 0x74.
 * @source 0x801AFA1C */
extern void func_801AFA1C(const u8 *arg0, u8 arg1, s16 arg2, s16 arg3,
                          s32 arg4, s32 arg5, s32 arg6);

/* @source 0x801D7BFC
 * @behavior Equipment category list panel of the SHISU overlay, the variant
 *   that shows unavailable entries instead of filtering them out. It paints the
 *   0x99 by 0x9A inset at (x plus 3, y plus 3) through func_801AE3F0 with the
 *   +0x09 byte as the shade and the shared CLUT-bank byte, asks func_801AFD10
 *   for the row window with the +0x0B byte, the two stack bytes and the +0x12
 *   halfword, and offsets both equipment lists D_801C893C and D_801C8950 of the
 *   +0x0A category by that answer. The returned visible row count plus nine
 *   rows are emitted 13 pixels apart from y plus the signed byte the query
 *   returned. A zero entry emits nothing. Every other entry is tested once:
 *   when the +0x09 byte is zero, func_801AF494 with the +0x08 byte, the
 *   character record D_80181B10[D_80144F5A[D_80148656]] and the category marks
 *   the entry unavailable when it answers zero; a nonzero +0x09 byte marks it
 *   unavailable outright. An unavailable entry takes the shade 7 and the row
 *   flag 1, an available one the shade 0 and the flag 0, and the shade becomes
 *   2 when the +0x0B byte plus the walk index meets either the +0x0D or the
 *   +0x0C byte plus the window offset, where that offset is zero when bits 4
 *   and up of the signed +0x12 halfword are 0xF and the visible row count
 *   otherwise. Each row carries the category byte and the entry id, and the
 *   byte drawn beside it is 1 for the category 4 and the parallel byte of
 *   D_801C8950 otherwise. An entry the +0x0D or the +0x0C byte selects is
 *   emitted twice, once at 0x1A below the row origin with the shade 7 and the
 *   row flag 1 when its shade is not already 7, and once at 0x18 below it with
 *   its own shade and its own flag; every other entry is emitted once at 0x1A
 *   with its own shade and flag. The 0x99 by 0x14 strip at (x plus 3, y plus 3)
 *   and the 0x99 by 8 strip at (x plus 3, y plus 0x92) follow with the same
 *   shade and CLUT byte, then the caption D_801D9E70[+0x0A], one of the runs
 *   ITEM, WEAPON, ARMOR, OPTION and VITAL, is right-aligned through
 *   func_801AF20C and drawn at (x plus 0x4E minus 4 times its length, y plus 7)
 *   with the shade 7 or 0 by the +0x09 byte and the glyph width 0x10. The
 *   counter func_80166438 answers for the category is formatted through sprintf
 *   into the shared buffer D_80145AD4 with the format "%3d/%3d" and the
 *   capacity 0x20 for the category 4 and 0x80 otherwise, and drawn through
 *   func_80150098 at (x plus 0x55, y plus 0x92) with the same shade. The two
 *   sprite streams D_801D9960 or D_801D9978 and D_801D996C or D_801D9984 then
 *   follow at the panel origin with the flag 1, selected by bits 0x02 and 0x01
 *   of the +0x10 halfword, which the body steps down by 0x10 while any bit of
 *   its high nibble is set and clears otherwise. It closes with the border
 *   pieces 1 along the top from x plus 40 to x plus 112, 4 down the left side
 *   from y plus 24 to y plus 136, 0x16 at (x plus 0x90, y plus 0x18), 0x17 down
 *   the right side from y plus 40 to y plus 128, 0x18 at (x plus 0x90, y plus
 *   0x88), 0x19 at (x, y plus 0x90), 0x1A along the bottom from x plus 8 to x
 *   plus 72, 0x1B at (x plus 0x50, y plus 0x90), 0x1C from x plus 0x58 to x
 *   plus 0x80, 0x1D at (x plus 0x88, y plus 0x90), and the two corner pieces
 *   0x32 at (x plus 8, y) and 0x33 at (x plus 0x90, y) only for the category 4,
 *   all with the flag 1, before the entry list D_801C893C[+0x0A], the +0x0B
 *   byte, the two coordinates (x plus 0x90, y plus 0x18) and the grid
 *   parameters 9, 0x20 or 0x80 and 0x74 reach func_801AFA1C. No jal and no word
 *   of this payload names the span, so an outside caller invokes it as an
 *   overlay entry. Its 0x50-byte frame keeps $s0 through $s7 and $ra, spills
 *   the two row-window bytes to 0x20($sp) and 0x21($sp), and the epilogue
 *   returns no value. One payload compare proved these 0x760 bytes equal the
 *   emi/etc/shop/00 copy at 0x801DC6FC and the emi/etc/bate/00 copy at
 *   0x801D6C54 apart from nine words each: the three internal j words that move
 *   with each copy's own address delta and the six low halves of this payload's
 *   caption-table and sprite-stream addresses, so the three are one body and
 *   those rows are shape evidence rather than a clone source. The border,
 *   stream and caption sequence is the one the sibling
 *   drawEquipCategoryListPanelShisu00_801D8A78 writes with its own stream
 *   choice and corner guard.
 * @status partial
 * @match 62.08
 * @residual 293 of 472 instructions at the original 1888-byte size. First
 *   mismatch at instruction 0: this candidate keeps the task pointer in $s0 and
 *   the walk index in $s1 where the original keeps the index in $s0 and the
 *   task in $s1, so every instruction that names either register differs while
 *   the surrounding code is equal. The frame is 0x58 instead of 0x50 because
 *   this candidate reserves 16 bytes of frame locals where the original
 *   reserves 8, although only the two row-window bytes at 0x20($sp) and
 *   0x21($sp) occupy either area. The inset paints, the row-window query, the
 *   two stack bytes, the outgoing argument slots, the availability test, the
 *   double-emission arm, the caption draw, the counter format, both sprite
 *   streams with the +0x10 step-down and the whole border sequence match.
 */
void drawEquipCategoryAvailabilityListPanelShisu00_801D7BFC(
    ShisuEquipCategoryAvailabilityTask *task) {
  u8 *rows;
  u8 *marks;
  u8 row;
  u8 rowCount;
  u32 rowY;
  u8 offset;
  u8 i;
  u32 flag;
  u8 grey;
  u8 shade;
  u8 sel;
  u8 pair;
  u32 len;
  u32 rowTop;

  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x99, 0x9A,
                task->unk_09, D_80144952);
  offset = (u8)func_801AFD10(&task->unk_0B, &row, &rowCount, &task->unk_12);
  rows = D_801C893C[task->unk_0A] + offset;
  marks = D_801C8950[task->unk_0A] + offset;
  rowY = task->y + (s8)row;
  i = 0;
  while ((u8)i < rowCount + 9) {
    if (*rows != 0) {
      flag = 1;
      if (task->unk_09 == 0) {
        flag = 0;
        if ((u8)func_801AF494(task->unk_08,
                              D_80181B10[D_80144F5A[(s8)D_80148656]],
                              task->unk_0A, *rows) == 0) {
          flag = 1;
        }
      }
      grey = flag;
      shade = (0 - grey) & 7;
      sel = ((s16)task->unk_12 >> 4) == 0xF ? 0 : rowCount;
      if (task->unk_0B + (u8)i == task->unk_0D + (u8)sel) {
        shade = 2;
      }
      pair = task->unk_0A == 4 ? 1 : *marks;
      if (task->unk_0B + (u8)i == task->unk_0D + (u8)sel ||
          task->unk_0B + (u8)i == task->unk_0C + (u8)sel) {
        if (shade != 7) {
          rowTop = rowY + 0x1A;
          func_801AF800((s16)(task->x + 7), (s16)rowTop, 7,
                        task->unk_0A, *rows, pair, 1);
        }
        rowTop = rowY + 0x18;
        func_801AF800((s16)(task->x + 7), (s16)rowTop, shade,
                      task->unk_0A, *rows, pair, grey);
      } else {
        rowTop = rowY + 0x1A;
        func_801AF800((s16)(task->x + 7), (s16)rowTop, shade,
                      task->unk_0A, *rows, pair, grey);
      }
    }
    rows++;
    marks++;
    rowY += 0xD;
    i++;
  }

  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x99, 0x14,
                task->unk_09, D_80144952);
  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 0x92), 0x99, 8,
                task->unk_09, D_80144952);

  len = (u8)func_801AF20C(D_801D9E70[task->unk_0A]);
  func_8014F800((s16)(task->x - (len * 4 - 0x4E)), (s16)(task->y + 7),
                (task->unk_09 != 0) ? 7 : 0, 0x10,
                (u32)D_801D9E70[task->unk_0A]);
  sprintf((char *)D_80145AD4, D_801D0D40,
          (u8)func_80166438(task->unk_0A),
          task->unk_0A == 4 ? 0x20 : 0x80);
  func_80150098((s16)(task->x + 0x55), (s16)(task->y + 0x92),
                (task->unk_09 != 0) ? 7 : 0, D_80145AD4);

  func_801AF390((s16)task->x, (s16)task->y,
                (task->unk_10 & 2) ? D_801D9978 : D_801D9960, 1);
  func_801AF390((s16)task->x, (s16)task->y,
                (task->unk_10 & 1) ? D_801D9984 : D_801D996C, 1);

  if (task->unk_10 & 0xF0) {
    task->unk_10 = task->unk_10 - 0x10;
  } else {
    task->unk_10 = 0;
  }

  i = 0;
  do {
    func_801AF2A0((s16)(task->x + (i + 5) * 8), (s16)task->y, 1, 1);
    i++;
  } while (i < 0xA);

  i = 0;
  do {
    func_801AF2A0((s16)task->x, (s16)(task->y + (i + 3) * 8), 4, 1);
    i++;
  } while (i < 0xF);

  i = 0;
  func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x18), 0x16, 1);
  do {
    func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + (i + 5) * 8), 0x17,
                  1);
    i++;
  } while (i < 0xC);

  i = 0;
  func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x88), 0x18, 1);
  func_801AF2A0((s16)task->x, (s16)(task->y + 0x90), 0x19, 1);
  do {
    func_801AF2A0((s16)(task->x + (i + 1) * 8), (s16)(task->y + 0x90), 0x1A,
                  1);
    i++;
  } while (i < 9);

  i = 0;
  func_801AF2A0((s16)(task->x + 0x50), (s16)(task->y + 0x90), 0x1B, 1);
  do {
    func_801AF2A0((s16)(task->x + (i + 0xB) * 8), (s16)(task->y + 0x90), 0x1C,
                  1);
    i++;
  } while (i < 6);
  func_801AF2A0((s16)(task->x + 0x88), (s16)(task->y + 0x90), 0x1D, 1);
  if (task->unk_0A == 4) {
    func_801AF2A0((s16)(task->x + 8), (s16)task->y, 0x32, 1);
    func_801AF2A0((s16)(task->x + 0x90), (s16)task->y, 0x33, 1);
  }

  func_801AFA1C(D_801C893C[task->unk_0A], task->unk_0B,
                (s16)(task->x + 0x90), (s16)(task->y + 0x18), 9,
                task->unk_0A == 4 ? 0x20 : 0x80, 0x74);
}
