#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One row record of the overlay that calls this body: five leading glyph codes
 * drawn as a single five-cell text run, three sprite kinds of the 0x28 by 0x30
 * emitter at +5, and the numeric fields the panel formats. Only the offsets this
 * body reads are named, and the roles no byte proves stay address anchored.
 */
typedef struct BateStatusRowRecord {
  u8 name[5];   /* +0x00 five glyph codes of the name run */
  u8 sprite[3]; /* +0x05 three sprite kinds, 0xFF leaves the slot empty */
  u8 unk_08;    /* +0x08 formatted "%2d" and handed to the gauge helper */
  u8 unk_09[3];
  u8 unk_0C; /* +0x0C first "%02d" field */
  u8 unk_0D; /* +0x0D second "%02d" field */
  u8 unk_0E[2];
  u32 unk_10; /* +0x10 trailing argument of the gauge helper */
  u8 unk_14;  /* +0x14 nonzero appends the 0x3D sprite */
} BateStatusRowRecord;

/* Shared CLUT-bank byte of the main executable that every BATE panel passes as
 * the trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared sprintf work buffer of the main executable, outside this EMI payload.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* PsyQ graph-type query, declared here because this translation unit reads only
 * its answer. */
extern s32 GetGraphType(void);

/* PsyQ formatted print into the shared work buffer. */
extern int sprintf(char *buffer, const char *format, ...);

/* Sprite emitter of this overlay, still an `asm` row of this target's Splat
 * layout: it takes the entry of the four-byte-stride descriptor table selected
 * by the kind byte and fills a 0x28 by 0x30 sprite from it. Only its call is
 * bound here, with the widths the call site widens its arguments to.
 * @source 0x801D3518 */
extern void func_801D3518(s32 x, s32 y, s32 kind, s32 mode);

/* Panel icon emitter of this overlay, a shared panel template wrapper that is
 * already lifted exact in this target. The original zero-extends both icon
 * coordinates (andi 0xFFFF), so this call site keeps the unsigned coordinates
 * the template's signed ones are widened from. */
void emitPanelIconPrimBate00_801D4678(u16 arg0, u16 arg1, s32 arg2, s32 arg3,
                                      u16 arg4, u8 arg5);

/* Text submitter of the main executable: the row position, the alignment and
 * the glyph-code row to queue. This payload only calls it.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8 *arg3);

/* Gauge helper of this overlay, still an `asm` row of this target's Splat
 * layout: the two coordinates, one selector word, the record byte at +0x08 and
 * the record word at +0x10.
 * @source 0x801D4730 */
extern void func_801D4730(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries. This payload only
 * calls it.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);

/* Sprite emitter of the main executable: it fills one GT quad from the
 * sprite-record table entry the id selects. This payload only calls it.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* The two format strings and the three-cell label row of this payload's
 * initialized blob: "%2d" at 0x801D0D80, "%02d %02d" at 0x801D0D84 and "  :" at
 * 0x801D0D90, all inside the payload rather than the main executable. */
extern const char D_801D0D80[]; /* @source 0x801D0D80 @kind data */
extern const char D_801D0D84[]; /* @source 0x801D0D84 @kind data */
extern const char D_801D0D90[]; /* @source 0x801D0D90 @kind data */

/* Sprite-record stream of this payload's initialized blob, submitted at the row
 * origin. */
extern const u8 D_801D8A3C[]; /* @source 0x801D8A3C @kind data */

/* @source 0x801D8208
 * @behavior Three-icon status row panel of the BATE overlay: takes the icon id,
 *           the row origin and one row record. It first insets the 200 by 48
 *           panel through func_801AE3F0 at (x + 8, y) with the shade 0 and the
 *           shared CLUT-bank byte D_80144952, and a null record skips the whole
 *           body below it. For a record it re-reads the graph type when that
 *           answer is not 1, installs the draw mode at g_PrimCursor with dfe 0,
 *           dtd 0, the texture page 0xF, no texture window and slot 1 as 0xC
 *           bytes, then emits the 16x16 icon through
 *           emitPanelIconPrimBate00_801D4678 at (x + 9, y + 5) with the icon id
 *           plus 3 as its kind, the 0x1E texture row, the clut GetClut(0x10,
 *           0x1E0) and the attribute 0x80. It then draws the record three sprite
 *           kinds: for every slot below 3 whose byte is not 0xFF it calls
 *           func_801D3518 at (x + 0x56 plus 41 per slot, y + 2) with that byte as
 *           the kind and the mode 0. The five-cell name run of the record is
 *           submitted through func_8014F800 at (x + 0x15, y + 3) with the
 *           alignment 0 and the count 5, then the record byte at +0x08 is
 *           formatted with "%2d" and drawn through func_80150098 at (x + 0x1F,
 *           y + 0x18), the record bytes +0x0C and +0x0D are formatted with
 *           "%02d %02d" and drawn at (x + 0x27, y + 0x25), and that same column
 *           draws the three-cell label row "  :" at y + 0x24 and y + 0x27. The
 *           record byte at +0x08 and its word at +0x10 then reach the gauge
 *           helper func_801D4730 at (x + 0x14, y + 0x12) with the selector 0.
 *           Outside the guarded block it reads the record byte at +0x14 and
 *           passes 2, or 0 when either that byte or the record is zero, as the
 *           flag of the sprite stream D_801D8A3C submitted through func_801AF390
 *           at the row origin as two signed halfwords, and when the record is
 *           present and its +0x14 byte is set it appends the sprite 0x3D through
 *           func_801AF2A0 at (x + 0x32, y + 0x16). No jal and no table word of
 *           this payload names this body, so it is an overlay entry called from
 *           outside the payload. The same 0x300-byte body sits at 0x801D91B0 in
 *           emi/etc/shisu/00, and a cmp of the two payload spans found 185 of
 *           their 192 words equal: the seven that differ are the three jal words
 *           to each copy own sprite emitter, panel icon emitter and gauge helper,
 *           which do not move with the copy delta, plus the four low halves of
 *           the two format strings, the label row and the stream blob, which
 *           carry each payload own blob base, so the exact lift of that twin
 *           supplied this statement order and name while every literal, callee
 *           and blob base here is written from this span own bytes. Its 0x40-byte
 *           frame keeps s0 through s7, $fp and $ra and the epilogue returns no
 *           value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThreeIconStatusRowPanelBate00_801D8208(u8 icon, s16 x, s16 y,
                                                BateStatusRowRecord *rec) {
  s32 origin_x;
  s32 origin_y;
  u8 i;
  u8 flag;

  func_801AE3F0((u16)(x + 8), (u16)y, 0xC8, 0x30, 0, D_80144952);
  if (rec != 0) {
    if (GetGraphType() != 1) {
      GetGraphType();
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, 0xF, 0);
    GpuAppendPrim(1, 0xC);
    emitPanelIconPrimBate00_801D4678(
        (u16)(x + 9), (u16)(y + 5), (u8)(icon + 3), 0x1E,
        (u16)GetClut(0x10, 0x1E0), 0x80);
    for (i = 0; i < 3; i++) {
      if (rec->sprite[i] != 0xFF) {
        func_801D3518((u16)(x + 0x56 + i * 41), (u16)(y + 2), rec->sprite[i], 0);
      }
    }
    func_8014F800((s16)(x + 0x15), (s16)(y + 3), 0, 5, (u32)rec);
    sprintf((char *)D_80145AD4, D_801D0D80, rec->unk_08);
    func_80150098((s16)(x + 0x1F), (s16)(y + 0x18), 0, D_80145AD4);
    sprintf((char *)D_80145AD4, D_801D0D84, rec->unk_0C, rec->unk_0D);
    func_80150098((s16)(x + 0x27), (s16)(y + 0x25), 0, D_80145AD4);
    func_80150098((s16)(x + 0x27), (s16)(y + 0x24), 0, D_801D0D90);
    func_80150098((s16)(x + 0x27), (s16)(y + 0x27), 0, D_801D0D90);
    func_801D4730((u16)(x + 0x14), (u16)(y + 0x12), 0, rec->unk_08, rec->unk_10);
  }
  do {
    origin_x = (s16)x;
    origin_y = (s16)y;
  } while (0);
  flag = 0;
  if (rec->unk_14 != 0) {
    flag = (rec != 0) << 1;
  }
  func_801AF390(origin_x, origin_y, D_801D8A3C, flag);
  if (rec != 0 && rec->unk_14 != 0) {
    func_801AF2A0((s16)(x + 0x32), (s16)(y + 0x16), 0x3D, 0);
  }
}
