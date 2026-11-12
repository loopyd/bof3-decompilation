#include "bof3/bof3.h"

/* Panel task root the caller hands this body. Its prefix repeats the shared
 * panel layout (the two origin halfwords at +0x04 and +0x06), the bytes the row
 * walk reads follow it and the marker halfword closes the run. Only the offsets
 * this body reads are named, and the roles no byte proves stay address
 * anchored.
 */
typedef struct ShisuAbilityListPanelTask {
  u8 unk_00[4];
  u16 x;        /* +0x04 panel origin column */
  u16 y;        /* +0x06 panel origin row */
  u8 unk_08;    /* +0x08 record byte handed to the selectable-entry test */
  u8 unk_09;    /* +0x09 sprite-stream selector byte, bits 0 and 1 */
  u8 unk_0A;    /* +0x0A first selector byte of the entry-list lookup */
  u8 unk_0B;    /* +0x0B second selector byte, also the caption index */
  u8 unk_0C;    /* +0x0C row index emitted as a highlighted row */
  u8 unk_0D;    /* +0x0D row index emitted as a highlighted row */
  u8 unk_0E[2]; /* +0x0E */
  u16 unk_10;   /* +0x10 nonzero adds the two corner pieces 0x32 and 0x34 */
} ShisuAbilityListPanelTask;

/* Shared CLUT-bank byte of the main executable that every SHISU panel passes as
 * the trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared work byte of the main executable read once per row: this body passes
 * it zero-extended to the selectable-entry test and sign-extended as the index
 * of the two mapping tables below, so the original loads it twice with two
 * different extensions.
 * @source 0x80148656 @kind data */
extern u8 D_80148656;

/* The two mapping tables of the main executable the row walk chains to reach a
 * character record: D_80181B10[D_80144F5A[character]] is that record index.
 * @source 0x80144F5A @kind table */
extern const u8 D_80144F5A[];
/* @source 0x80181B10 @kind table */
extern const u8 D_80181B10[];

/* The ability-object bank of the main executable at the shared header address
 * 0x801CA70C, bound here as a byte base because this body only forms the
 * address of the 20-byte entry the row's kind byte selects.
 * @source 0x801CA70C @kind table */
extern const u8 D_801CA70C[];

/* The category caption table of this payload's trailing blob: the four pointers
 * to the runs "HEAL", "ASSIST", "ATTACK" and "SKILL", selected by the task byte
 * at +0x0B. The measured caption is right-aligned inside the panel through
 * strlen, so the entries are byte strings.
 * @source 0x801D9EA4 @kind table */
extern const u8 *D_801D9EA4[];

/* The two sprite-record streams of the same blob, each submitted at the panel
 * origin with the flag 1: the first pair is selected by bit 0x02 of the task
 * byte at +0x09 and the second pair by bit 0x01.
 * @source 0x801D9960 @kind data */
extern const u8 D_801D9960[];
/* @source 0x801D9978 @kind data */
extern const u8 D_801D9978[];
/* @source 0x801D9990 @kind data */
extern const u8 D_801D9990[];
/* @source 0x801D999C @kind data */
extern const u8 D_801D999C[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Entry-list lookup of the main executable: the two selector bytes of the panel
 * task and a zero third argument return the byte run this body walks, whose
 * entries are the ability kinds of the panel's list.
 * @source 0x80166AE4 */
extern u8 *func_80166AE4(u8 arg0, u8 arg1, u32 arg2);

/* Selectable-entry test of the main executable. This call site fills only the
 * two leading argument positions, and it reads the answer through its low byte
 * alone, so the widths here are the call-site widths.
 * @source 0x801AF5B0 */
extern s32 func_801AF5B0(u8 arg0, u8 arg1);

/* Ability-kind icon query of the main executable; only its low byte reaches the
 * row emitter.
 * @source 0x801660DC */
extern u32 func_801660DC(u8 arg0);

/* Required-level query of the main executable: the character record index, the
 * ability kind and the mode byte. Only its low byte reaches the row emitter.
 * @source 0x80166A10 */
extern u32 func_80166A10(u8 arg0, u8 arg1, u32 arg2);

/* Row emitter of the main executable: the two signed coordinates, the shade,
 * the ability-kind icon byte, the ability-object entry, that entry's level byte
 * and the row flag. Every role here is positional, since no lifted body of this
 * repository names the callee.
 * @source 0x801AF930 */
extern void func_801AF930(s16 arg0, s16 arg1, s32 arg2, u8 arg3,
                          const u8 *arg4, u8 arg5, s32 arg6);

/* Text submitter of the main executable: the pen origin, the alignment, the
 * cell count and the glyph-code run it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);

/* Sprite emitter of the main executable: it fills one GT quad from the
 * sprite-record table entry the id selects and appends it at slot 1.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* PsyQ libc string length, called only for its answer here. */
extern int strlen(const char *arg0);

/* @source 0x801D835C
 * @behavior Ability list panel of the SHISU overlay: it fills the 0x9A by 0xA0
 *   inset rectangle at (x+3, y+3) through func_801AE3F0 with the shade 0 and
 *   the shared CLUT-bank byte D_80144952, takes the byte run that
 *   func_80166AE4 returns for the task bytes at +0x0A and +0x0B, and walks its
 *   first ten entries. A zero entry emits nothing; every other entry is one
 *   ability kind, and its row is emitted at x+7 and 13 pixels below the panel
 *   origin per walk step: the selectable-entry test func_801AF5B0(+0x08 byte,
 *   D_80148656) is read through its low byte, giving the shade 7 when that byte
 *   is zero and 0 otherwise, overridden to 2 on the row the +0x0D byte names.
 *   The row then carries the icon func_801660DC returns for the kind, the
 *   ability-object entry of the shared bank D_801CA70C at 20 bytes per kind, and
 *   the level func_80166A10(character index, kind, 0) reports, where the
 *   character index is D_80181B10[D_80144F5A[D_80148656]]; all three reach
 *   func_801AF930 as its icon, entry and level arguments. The rows the +0x0D and
 *   +0x0C bytes name are emitted twice - once at 0x1A below the row origin with
 *   the shade 7 and the row flag 1 when the computed shade is not 7, and once at
 *   0x18 below it with the computed shade - while every other row is emitted
 *   once at 0x1A with its own shade. The caption is the string
 *   D_801D9EA4[+0x0B] - one of the runs HEAL, ASSIST, ATTACK and SKILL -
 *   right-aligned through strlen and drawn at (x+0x4E-4*strlen, y+7) with the
 *   alignment 0 and the glyph width 0x10. The two sprite streams D_801D9960 or
 *   D_801D9978 and D_801D9990 or D_801D999C follow at the panel origin with the
 *   flag 1, selected by bits 0x02 and 0x01 of the +0x09 byte, which the body
 *   then steps down by 0x10 while any bit of its high nibble is set and clears
 *   otherwise. It closes with the border pieces 1 along the top from x+40 to
 *   x+112, 4 down the left side from y+24 to y+152, 0x20 at (x+0x90, y+0x18),
 *   0x21 down the right side from y+32 to y+144, 5 at (x, y+0xA0), 0x22 at
 *   (x+0x90, y+0x98), the two top corner pieces 0x32 at (x+8, y) and 0x34 at
 *   (x+0x90, y) only when the halfword at +0x10 is not zero, and 6 along the
 *   bottom from x+8 to x+136, all with the flag 1. No jal and no word of this
 *   payload names the span, so an outside caller invokes it as an overlay entry;
 *   its 0x58-byte frame keeps s0 through s7, $fp and $ra, spills the
 *   selectable-row flag to 0x20($sp) and the epilogue returns no value. One
 *   payload cmp proved these 0x4F4 bytes equal the emi/etc/shop/00 copy at
 *   0x801DCE5C apart from eight words, and the emi/etc/bate/00 copy at
 *   0x801D73B4 differs in those same eight positions: the two internal j words
 *   that move with each copy's own address delta and the six low halves of this
 *   payload's caption-table and sprite-stream addresses. The three are one body,
 *   so those rows are shape evidence and not a clone source, and the border,
 *   stream and caption sequence is the one drawMastersCommandPanelShisu00_801D3320
 *   writes with the panel width 0x95.
 * @status partial
 * @match 77.64
 * @residual 257 of 317 instructions at 1324 bytes against the original 1268. First
 *   mismatch at instruction 0: this candidate keeps the row origin in a 0x58-kind
 *   frame of 0x60 bytes because the compiler turns the two (s16)(rowY + 0x1A)
 *   arguments into a shifted derived induction variable, spills the plain
 *   accumulator to 0x20($sp) and keeps the selectable-row flag in s7, where the
 *   original keeps the accumulator in s7, truncates per call site with addiu plus
 *   sll and sra, and puts the flag in 0x20($sp); the caption, the two sprite
 *   streams, the byte decrement and the border-piece walk already match
 *   instruction for instruction. Measured and rejected on this allocation: a
 *   source-level do/while accumulator of type u16, s16, s32 and u32, the
 *   (u32)task->y + (i * 13 + K) form with and without a hoisted origin local,
 *   list[i] indexing in place of the walked pointer, the entry assignment before
 *   and after the level query, a shared s16 row local, unsigned shade
 *   arithmetic, block-scope row locals, and a mode variable against repeated
 *   (u8)shade masks, all of which measure at or below this candidate.
 */
void drawAbilityListPanelShisu00_801D835C(ShisuAbilityListPanelTask *task) {
  u8 *p;
  u8 *entry;
  u32 icon;
  u32 count;
  u32 dim;
  u8 i;
  u8 shade;
  u32 rowY;

  func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x9A, 0xA0, 0,
                D_80144952);
  p = func_80166AE4(task->unk_0A, task->unk_0B, 0);
  rowY = task->y;
  i = 0;
  do {
    if (*p != 0) {
      dim = (u8)func_801AF5B0(task->unk_08, D_80148656) < 1;
      shade = -dim & 7;
      if (i == task->unk_0D) {
        shade = 2;
      }
      icon = func_801660DC(*p);
      entry = (u8 *)(D_801CA70C + *p * 20);
      count = func_80166A10(D_80181B10[D_80144F5A[(s8)D_80148656]], *p, 0);
      if (i == task->unk_0D || i == task->unk_0C) {
        if ((u8)shade != 7) {
          func_801AF930((s16)(task->x + 7), (s16)(rowY + 0x1A), 7, icon, entry,
                        count, 1);
        }
        func_801AF930((s16)(task->x + 7), (s16)(rowY + 0x18), (u8)shade, icon, entry,
                      count, dim);
      } else {
        func_801AF930((s16)(task->x + 7), (s16)(rowY + 0x1A), shade, icon, entry,
                      count, dim);
      }
    }
    i++;
    p++;
    rowY += 0xD;
  } while (i < 0xA);

  func_8014F800(
      (s16)(task->x + 0x4E - strlen((const char *)D_801D9EA4[task->unk_0B]) * 4),
      (s16)(task->y + 7), 0, 0x10, (u32)D_801D9EA4[task->unk_0B]);
  func_801AF390((s16)task->x, (s16)task->y,
                (task->unk_09 & 2) ? D_801D9978 : D_801D9960, 1);
  func_801AF390((s16)task->x, (s16)task->y,
                (task->unk_09 & 1) ? D_801D999C : D_801D9990, 1);
  if (task->unk_09 & 0xF0) {
    task->unk_09 -= 0x10;
  } else {
    task->unk_09 = 0;
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
  } while (i < 0x11);

  func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x18), 0x20, 1);
  i = 0;
  do {
    func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + (i + 4) * 8), 0x21, 1);
    i++;
  } while (i < 0xF);
  func_801AF2A0((s16)task->x, (s16)(task->y + 0xA0), 5, 1);
  func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x98), 0x22, 1);
  if (task->unk_10 != 0) {
    func_801AF2A0((s16)(task->x + 8), (s16)task->y, 0x32, 1);
    func_801AF2A0((s16)(task->x + 0x90), (s16)task->y, 0x34, 1);
  }
  i = 0;
  do {
    func_801AF2A0((s16)(task->x + (i + 1) * 8), (s16)(task->y + 0xA0), 6, 1);
    i++;
  } while (i < 0x11);
}
