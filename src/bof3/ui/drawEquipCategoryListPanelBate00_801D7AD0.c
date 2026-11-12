#include "bof3/bof3.h"

/* Panel task root the outside caller hands this body. Its prefix repeats the
 * shared panel layout (the two origin halfwords at +0x04 and +0x06); the byte
 * this body derives the equipment category into follows, then the selector
 * bytes of the row walk and the byte the selected entry is published in. Only
 * the offsets this body touches are named, and the roles no byte proves stay
 * address anchored.
 */
typedef struct BateEquipCategoryListTask {
  u8 unk_00[4];
  u16 x;         /* +0x04 panel origin column */
  u16 y;         /* +0x06 panel origin row */
  u8 category;   /* +0x08 category this body derives from +0x09, and writes */
  u8 state;      /* +0x09 byte the leading switch dispatches on */
  u8 unk_0A;     /* +0x0A first index of the visible row window */
  u8 unk_0B;     /* +0x0B index of the selected entry */
  u8 unk_0C;     /* +0x0C character record byte of the usability test */
  u8 unk_0D;     /* +0x0D entry id this body publishes */
  u8 unk_0E[2];  /* +0x0E */
  u16 unk_10;    /* +0x10 halfword the row-window query reads and writes */
} BateEquipCategoryListTask;

/* Shared CLUT-bank byte of the main executable that every BATE panel passes as
 * the trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* The two mapping tables of the main executable the usability test chains to
 * reach a character record: D_80181B10[D_80144F5A[task byte +0x0C]] is that
 * record index. Both are work RAM of the main executable, and D_80144F5A holds
 * no initialized byte in the image.
 * @source 0x80144F5A @kind table */
extern const u8 D_80144F5A[];
/* @source 0x80181B10 @kind table */
extern const u8 D_80181B10[];

/* The two parallel equipment lists of the main executable, selected by the
 * category byte this body derives: D_801C893C carries the entry ids the panel
 * walks and D_801C8950 the byte the row emitter draws beside each id. Both are
 * work RAM of the main executable and hold no initialized word in the image.
 * @source 0x801C893C @kind table */
extern u8 *D_801C893C[];
/* @source 0x801C8950 @kind table */
extern u8 *D_801C8950[];

/* Shared sprintf work buffer of the main executable: the entry counter is
 * formatted here and then handed to the line-text drawer.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* The counter format of this overlay's leading data blob at payload byte 0x15C:
 * the run "%3d/%3d", which this body fills with the collected entry count and
 * the 128-entry list bound.
 * @source 0x801D0D5C @kind data */
extern const char D_801D0D5C[];

/* Caption table of this overlay's trailing blob, indexed by the category byte:
 * its five entries point at the runs ITEM, WEAPON, ARMOR, OPTION and VITAL that
 * sit at payload bytes 0x8290 through 0x82B0. The selected run is measured with
 * func_801AF20C and drawn right-aligned inside the panel.
 * @source 0x801D8EB8 @kind table */
extern const u8 *D_801D8EB8[];

/* The two 128-byte filter result lists of this overlay's trailing blob: the
 * walk below fills them with the entry ids and the parallel bytes of the
 * equipment lists whose entries a character may use, then the visible row
 * window is drawn from the offset the row-window query returns. The cleared
 * tail of both lists and the entries beyond the row window stay zero.
 * @source 0x801D8F48 @kind data */
extern u8 D_801D8F48[];
/* @source 0x801D8FC8 @kind data */
extern u8 D_801D8FC8[];

/* The two 0xC-byte sprite-record streams of the same blob, each submitted at
 * the panel origin with the flag 1.
 * @source 0x801D893C @kind data */
extern const u8 D_801D893C[];
/* @source 0x801D8948 @kind data */
extern const u8 D_801D8948[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Entry-row emitter of the main executable: the two signed coordinates, the
 * shade, the category byte, the entry id, the byte drawn beside it and the row
 * flag. Only the positions this call site fills are evidenced, so every role
 * stays positional.
 * @source 0x801AF800 */
extern void func_801AF800(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4,
                          u8 arg5, s32 arg6);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries. This payload only
 * calls it.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Sprite emitter of the main executable: it fills one GT quad from the
 * sprite-record table entry the id selects and appends it at slot 1. This
 * payload only calls it.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* Glyph-run submitter of the main executable: the two signed coordinates, the
 * alignment, the cell count and the address of the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * alignment argument and the string buffer it draws.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* String measure of the main executable: it answers the length of the caption
 * run, which this body reads through its low byte and scales by four cells.
 * @source 0x801AF20C */
extern u32 func_801AF20C(const void* arg0);

/* Usability test of the main executable: the table selector 4, the character
 * record index, the category byte and the entry id, answered through its low
 * byte. A zero answer drops the entry from the filter result lists.
 * @source 0x801AF494 */
extern u32 func_801AF494(s32 arg0, u8 arg1, u8 arg2, u8 arg3);

/* Entry-class query of the main executable: the class selector 2 and the entry
 * id, answered through its low byte. This body compares that answer, offset by
 * two, with the state byte.
 * @source 0x80165E04 */
extern u32 func_80165E04(s32 arg0, u8 arg1);

/* Row-window query of the main executable: the first-index byte of the task,
 * the two work bytes it fills and the task halfword it reads, answered with the
 * offset of the first visible entry inside the filter result lists.
 * @source 0x801AFD10 */
extern u32 func_801AFD10(u8* arg0, u8* arg1, u8* arg2, u16* arg3);

/* Icon-grid emitter of the main executable: the entry list, the first-index
 * byte, the two signed coordinates and the three grid parameters 9, 0x80 and
 * 0x74.
 * @source 0x801AFA1C */
extern void func_801AFA1C(const u8* arg0, u8 arg1, s16 arg2, s16 arg3,
                          s32 arg4, s32 arg5, s32 arg6);

/* PsyQ libc formatter, called for its store into the shared buffer. */
extern int sprintf(char* buffer, const char* format, ...);

/* @source 0x801D7AD0
 * @behavior Equipment category list panel of the BATE overlay: it derives the
 *   category byte 1, 2 or 3 from the task state byte at +0x09 - the states 1,
 *   2 and 3 give 2, the states 4 and 5 give 3, and every other state gives 1 -
 *   writes it to the task byte at +0x08, and then filters the two parallel
 *   equipment lists D_801C893C[category] and D_801C8950[category] over their
 *   128 entries. An entry is kept when the usability test
 *   func_801AF494(4, D_80181B10[D_80144F5A[task byte +0x0C]], category, id) is
 *   non-zero, and, for the category 2 alone, when the entry-class query
 *   func_80165E04(2, id) offset by two equals the state byte minus one; the kept
 *   id and the parallel byte land in D_801D8F48 and D_801D8FC8 at the running
 *   index, counting the kept entries, and the rest of both 128-byte lists is
 *   cleared. The task byte at +0x0D then receives D_801D8F48[task byte +0x0B],
 *   the selected entry id. The 0x99 by 0x9A inset at (x+3, y+3) is painted with
 *   the shade 0 and the shared CLUT-bank byte D_80144952, the row window is
 *   queried through func_801AFD10 with the +0x0A byte, the two stack bytes and
 *   the +0x10 halfword, and its answer offsets both filter result lists. The
 *   returned visible row count plus nine rows are then emitted 13 pixels apart
 *   from y + (signed byte the query returned): a zero entry emits nothing, the
 *   row the +0x0A byte plus the row index selects gains a shadow emission at
 *   0x1A below the row origin with the shade 7 and the row flag 1 before its own
 *   emission at 0x18 with the shade 0, and every other row is emitted once at
 *   0x1A with the shade 0; all of them carry the category byte and the pair of
 *   list bytes. The panel then gains the 0x99 by 0x14 inset at (x+3, y+3) and
 *   the 0x99 by 8 inset at (x+3, y+0x92), both with the shade 0 and the shared
 *   CLUT-bank byte, the caption D_801D8EB8[category] - one of the runs ITEM,
 *   WEAPON, ARMOR, OPTION and VITAL - right-aligned through func_801AF20C and
 *   drawn at (x+0x4E-4*length, y+7) with the alignment 0 and the glyph width
 *   0x10, and the kept-entry counter formatted through sprintf into the shared
 *   buffer D_80145AD4 with the format "%3d/%3d" and the bound 128, drawn through
 *   func_80150098 at (x+0x55, y+0x92) with the alignment 0. The two sprite
 *   streams D_801D893C and D_801D8948 follow at the panel origin with the flag
 *   1, and the body closes with the border pieces 1 along the top from x+40 to
 *   x+112, 4 down the left side from y+24 to y+136, 0x16 at (x+0x90, y+0x18),
 *   0x17 down the right side from y+40 to y+128, 0x18 at (x+0x90, y+0x88), 0x19
 *   at (x, y+0x90), 0x1A along the bottom from x+8 to x+72, 0x1B at (x+0x50,
 *   y+0x90), 0x1C from x+0x58 to x+0x80, 0x1D at (x+0x88, y+0x90) and the two
 *   corner pieces 0x32 at (x+8, y) and 0x33 at (x+0x90, y), all with the flag 1,
 *   before the entry list D_801D8F48, the +0x0A byte, the two coordinates
 *   (x+0x90, y+0x18) and the grid parameters 9, 0x80 and 0x74 reach
 *   func_801AFA1C. No jal and no word of this payload names the span, so an
 *   outside caller invokes it as an overlay entry; its 0x48-byte frame keeps $s0
 *   through $s6 and $ra and the epilogue returns no value. One payload compare
 *   proved these 0x738 bytes equal the emi/etc/shisu/00 copy at 0x801D8A78 and
 *   the emi/etc/shop/00 copy at 0x801DD578 apart from 17 words each: the four
 *   internal j words, which move by exactly the address delta between the two
 *   spans, and this payload's own jump table and trailing-blob addresses
 *   (jtbl_801D0D68, the two filter lists, the caption table, the counter format
 *   and the two sprite streams), so the three are one body and those rows are
 *   shape evidence, not a clone source.
 * @status partial
 * @match 97.62
 * @residual 11 of 462 instructions differ at the original 1848-byte size.
 *   First mismatch at instruction 95: the original zeroes the row-walk counter
 *   into $s1 and this candidate into $s4, and the same swap exchanges $s1 and
 *   $s4 between that counter and the filtered mark-list pointer through the row
 *   loop, with the three later border loops' counter reset landing two
 *   instructions earlier here. Frame, size, jump table, filter walk, both inset
 *   groups, caption, counter format, stream and border emissions and the tail
 *   call are equal. The counter as its own variable rather than a reuse of the
 *   filter index (`rowIdx`) is what moved the task pointer from $s1 to the
 *   original's $s2, a signed 16-bit row base (`s16 rowY`) is what removed the
 *   shifted row-coordinate induction variable the original re-derives per call,
 *   and the held address of the shared CLUT-bank byte for the two later insets
 *   (`clutp`) is what restored the original's one-time address materialisation;
 *   the same body with a shared filter/row counter measured 359, with an
 *   unsigned 32-bit row base 422 and with a 32-bit row counter 428 of 462.
 */
void drawEquipCategoryListPanelBate00_801D7AD0(
    BateEquipCategoryListTask* task) {
  u8* src;
  u8* dst;
  u8* rows;
  u8* marks;
  u8 offset;
  u8 i;
  u8 rowIdx;
  u8 count;
  u8 n;
  u8 row;
  u8 rowCount;
  s16 rowY;
  u32 len;
  u8 idx;
  u8* clutp;

  switch (task->state) {
  case 0:
  default:
    task->category = 1;
    break;
  case 1:
  case 2:
  case 3:
    task->category = 2;
    break;
  case 4:
  case 5:
    task->category = 3;
    break;
  }
  i = 0;
  count = 0;
  n = 0;
  src = D_801C893C[task->category];
  dst = D_801C8950[task->category];
  do {
    if ((u8)func_801AF494(4, D_80181B10[D_80144F5A[task->unk_0C]],
                          task->category, *src) != 0) {
      if (task->category != 2 ||
          task->state - 1 == (u8)func_80165E04(2, *src) - 2) {
        count++;
        D_801D8F48[i] = *src;
        D_801D8FC8[i] = *dst;
        i++;
      }
    }
    src++;
    dst++;
    n++;
  } while (n < 0x80);

  while (i < 0x80) {
    D_801D8F48[i] = 0;
    D_801D8FC8[i] = 0;
    i++;
  }

  task->unk_0D = D_801D8F48[task->unk_0B];
  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x99, 0x9A, 0,
                D_80144952);
  offset = (u8)func_801AFD10(&task->unk_0A, &row, &rowCount, &task->unk_10);
  rows = D_801D8F48 + offset;
  marks = D_801D8FC8 + offset;
  rowY = task->y + (s8)row;
  rowIdx = 0;
  while ((u8)rowIdx < rowCount + 9) {
    if (*rows != 0) {
      if (task->unk_0A + rowIdx == task->unk_0B) {
        func_801AF800((s16)(task->x + 7), (s16)(rowY + 0x1A), 7,
                      task->category, *rows, *marks, 1);
        func_801AF800((s16)(task->x + 7), (s16)(rowY + 0x18), 0,
                      task->category, *rows, *marks, 0);
      } else {
        func_801AF800((s16)(task->x + 7), (s16)(rowY + 0x1A), 0,
                      task->category, *rows, *marks, 0);
      }
    }
    rows++;
    marks++;
    rowY += 0xD;
    rowIdx++;
  }

  clutp = &D_80144952;
  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x99, 0x14, 0, *clutp);
  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 0x92), 0x99, 8, 0, *clutp);

  idx = task->category;
  len = (u8)func_801AF20C(D_801D8EB8[idx]);
  func_8014F800((s16)(task->x - (len * 4 - 0x4E)), (s16)(task->y + 7), 0, 0x10,
                (u32)D_801D8EB8[idx]);
  sprintf((char*)D_80145AD4, D_801D0D5C, count, 0x80);
  func_80150098((s16)(task->x + 0x55), (s16)(task->y + 0x92), 0, D_80145AD4);

  func_801AF390((s16)task->x, (s16)task->y, D_801D893C, 1);
  func_801AF390((s16)task->x, (s16)task->y, D_801D8948, 1);

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
    func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + (i + 5) * 8), 0x17, 1);
    i++;
  } while (i < 0xC);

  i = 0;
  func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x88), 0x18, 1);
  func_801AF2A0((s16)task->x, (s16)(task->y + 0x90), 0x19, 1);
  do {
    func_801AF2A0((s16)(task->x + (i + 1) * 8), (s16)(task->y + 0x90), 0x1A, 1);
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
  func_801AF2A0((s16)(task->x + 8), (s16)task->y, 0x32, 1);
  func_801AF2A0((s16)(task->x + 0x90), (s16)task->y, 0x33, 1);

  func_801AFA1C(D_801D8F48, task->unk_0A, (s16)(task->x + 0x90),
                (s16)(task->y + 0x18), 9, 0x80, 0x74);
}
