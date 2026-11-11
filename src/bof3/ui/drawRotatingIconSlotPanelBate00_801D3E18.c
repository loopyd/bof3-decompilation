#include "bof3/bof3.h"
#include "gpu/prim.h"

/* The three icon kinds of the panel slots, in one 3-byte object so the panel
 * can take them by one assignment.
 */
typedef struct BateIconKinds {
    s8 kind[3]; /* +0x00 icon kinds 0, 1 and 2 */
} BateIconKinds;

/* One (x, y) pair of the panel's working tables. The rotating marker offsets of
 * the payload's data blob and the two scratch tables the panel fills are arrays
 * of this pair.
 */
typedef struct BateIconSlotOffset {
    s32 x; /* +0x00 */
    s32 y; /* +0x04 */
} BateIconSlotOffset;

/* One entry of the ten-entry label table the panel indexes with its selector
 * byte: the cells of the entry label, the number of cells that label occupies
 * and the (x, y) offset of each of the three icon slots of the entry. The 0x14
 * bytes of the entry are what the panel's record stride multiplies.
 */
typedef struct BateIconSlotRecord {
    u8 label[7];      /* +0x00 ASCII cells of the label, not terminated */
    u8 labelLen;      /* +0x07 cells of label the text drawer is given */
    s16 offset[3][2]; /* +0x08 (x, y) offset of each icon slot */
} BateIconSlotRecord;

/* The ten label records as one object, so the panel can take them by one
 * assignment. 10 * 0x14 = 0xC8 bytes.
 */
typedef struct BateIconSlotTable {
    BateIconSlotRecord entry[10]; /* +0x00 */
} BateIconSlotTable;

/* The label table of this payload's initialised blob: the ten entries are
 * "Normal", "Normal", "Attack", "Defense", "Normal", "Attack", "Defense",
 * "Chain", "Magic" and "Refuge", and the byte after each label is that label's
 * own length.
 * @source 0x801D0C50 @kind data */
extern BateIconSlotTable D_801D0C50;

/* The three icon kinds of the panel's slots, stored as signed bytes right after
 * the label table: 0, 1 and 2, so the panel draws the kinds three, four and
 * five of the icon emitter's atlas.
 * @source 0x801D0D18 @kind table */
extern BateIconKinds D_801D0D18;

/* The four (x, y) model offsets of the rotating backdrop marker, in the
 * payload's trailing blob: (0, 0), (-12, -1), (-4, 12) and (4, 12). The panel
 * rotates the first three into its own scratch table and draws them as one
 * Gouraud triangle.
 * @source 0x801D8B9C @kind data */
extern BateIconSlotOffset D_801D8B9C[];

/* The panel's two three-entry scratch tables in the same RAM blob: the rotated
 * backdrop-marker vertices and the rotated icon-slot offsets. Their bytes are
 * zero on disc.
 * @source 0x801D8F18 @kind data */
extern BateIconSlotOffset D_801D8F18[3];

/* @source 0x801D8F30 @kind data */
extern BateIconSlotOffset D_801D8F30[3];

/* The three sprite-record streams the panel submits at its own origin: the
 * backdrop cluster, one six-cell row walked in eight-pixel steps and the closing
 * stream. They lie in the same initialised blob and are only ever submitted.
 * @source 0x801D8898 @kind unknown */
extern const u8 D_801D8898[];

/* @source 0x801D88BC @kind unknown */
extern const u8 D_801D88BC[];

/* @source 0x801D88C8 @kind unknown */
extern const u8 D_801D88C8[];

/* Shared CLUT-bank byte of the main executable that this panel passes as the
 * trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared render-flags word of the main executable, outside this EMI payload: bit
 * 3 selects the direction its two-bit shading field in bits 1 and 2 is read in,
 * and that field's doubled value forms the flat colour of a lit icon slot.
 * @source 0x80143E6C @kind data */
extern s32 D_80143E6C;

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Text submitter of the main executable: the row position, the alignment, the
 * number of cells and the cell run. This payload only calls it.
 * @source 0x8014FC90 */
extern void func_8014FC90(s16 arg0, s16 arg1, s32 arg2, u32 arg3,
                          const u8 *arg4);

/* Primitive-append helper of the main executable: the ordering-table slot and
 * the size of the primitive at g_PrimCursor.
 * @source 0x8014E5A0 */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries. This payload only
 * calls it.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);

/* The two rotation helpers of the main executable: phase in, fixed-point cosine
 * and sine out. They are not lifted, so only the calls are bound here.
 * @source 0x801783C8 */
extern s32 func_801783C8(s32 phase);

/* @source 0x801782FC */
extern s32 func_801782FC(s32 phase);

/* Primitive-stamp helper of the main executable that the main RAM map leaves
 * unnamed: it stamps the packet it is given as the Gouraud triangle the panel
 * fills. This payload only calls it.
 * @source 0x8017A97C */
extern void func_8017A97C(void *arg0);

/* Panel icon emitter of this overlay, a shared panel template wrapper that is
 * already lifted exact in this target. The original zero-extends both icon
 * coordinates, so this call site keeps the unsigned coordinates the template's
 * signed ones are widened from. */
void emitPanelIconPrimBate00_801D4678(u16 arg0, u16 arg1, s32 arg2, s32 arg3,
                                      u16 arg4, u8 arg5);

/* @source 0x801D3E18
 * @behavior Rotating icon-slot panel of the BATE overlay. It copies the
 *           payload's ten-entry label table into its frame, loads the three
 *           icon kinds 0, 1 and 2 beside it, paints the 0x45 by 0x30 inset
 *           panel at (x + 3, y + 3) through func_801AE3F0 with the shade 0 and
 *           the shared CLUT-bank byte D_80144952, and takes the record the
 *           selector byte indexes with a 0x14-byte stride. The record label is
 *           submitted through func_8014FC90 at (x + 0x27 - labelLen * 4,
 *           y + 0x28) as a labelLen-cell run with the alignment 0, so it is
 *           right aligned on the panel. The slot count is 1 for the selector 0,
 *           2 for the selectors 1 through 3 and 3 for every other selector. The
 *           body then rotates the payload's four model offsets through
 *           func_801783C8 and func_801782FC, keeping the first three as its
 *           backdrop-marker vertices in D_801D8F18 with the horizontal term
 *           shifted down by 11 and the vertical term by 12. Those vertices fill
 *           one Gouraud triangle taken from g_PrimCursor and stamped by
 *           func_8017A97C: the three vertex pairs at (x + 0x26, y + 0x17) plus
 *           the rotated offsets, the red and green bytes of each vertex set to
 *           that vertex's rotated y shifted left by three and biased by -0x61,
 *           the blue bytes zero, and the 0x1C-byte packet appended at slot 1
 *           through func_8014E5A0. It then fills one colour byte per slot from
 *           the low bits of the colour-enable argument: a set bit takes the flat
 *           colour 0x3F | (((D_80143E6C & 8) ? (D_80143E6C & 6) :
 *           (~D_80143E6C & 6)) << 5) and a clear bit the grey 0x70. In the same
 *           walk it rotates the record's slot offsets through the same helpers
 *           into D_801D8F30 with the same two shifts, then sorts the slots by
 *           their rotated y with a bubble sort that swaps the rotated pair, the
 *           icon kind and the colour through signed-byte temporaries. Finally it
 *           emits one 16x16 icon per slot through emitPanelIconPrimBate00_801D4678
 *           at (x + 0x27 + slot x - 4, y + 0x17 + slot y - 8) with the icon kind
 *           plus 3, the 0x1E texture row, the clut GetClut(0x10, 0x1E0) and the
 *           slot's colour byte, and submits the three sprite streams D_801D8898
 *           at (x, y), D_801D88BC six times from x in eight-pixel steps and
 *           D_801D88C8 at (x, y) through func_801AF390 with the flag 0. No jal
 *           and no word of this payload names this span, so it is an overlay
 *           entry called from outside the payload; its 0x130-byte frame keeps
 *           $s0 through $s7, $fp and $ra and the epilogue returns no value. One
 *           payload cmp of these 0x74C bytes against
 *           drawRotatingIconSlotPanelShisu00_801D4DC0 at 0x801D4DC0 in
 *           emi/etc/shisu/00 and func_801D98C0 at 0x801D98C0 in emi/etc/shop/00
 *           found 428 of the 467 words equal and every other word an absolute
 *           one - the payload-local data halves, the displaced jal and internal
 *           j words - so the three spans are one body and those two still-`asm`
 *           rows are shape evidence rather than a clone source.
 * @status partial
 * @match 85.01
 * @residual The body matches at the original 0x74C extent and byte size; 397 of
 *           the 467 instructions are equal. The first difference is the entry
 *           save group: the original holds the phase argument in $s7 and the
 *           palette-base pointer in $s5, while this build holds them in $s5
 *           and $s7. The same ordering fact trades the slot count and the
 *           palette bit between $s4 and $s6, so the sort's four address bases
 *           and the row-origin halfword reloads land one register higher.
 *           Sampling each vertex shade before clearing its blue byte preserves
 *           the original load/store order. The remaining entry allocation
 *           difference is unresolved.
 */
void drawRotatingIconSlotPanelBate00_801D3E18(u16 x, u16 y, u8 index, u8 mask,
                                             s32 angle)
{
    BateIconSlotTable table;
    BateIconSlotRecord *record;
    POLY_G3 *marker;
    BateIconKinds iconId;
    u8 channels[3];
    u8 *colours;
    u8 lit;
    u8 count;
    u32 bit;
    u8 i;
    u8 j;
    s32 flags;
    s32 shade;
    s8 swap;

    table = D_801D0C50;
    colours = channels;
    iconId = D_801D0D18;
    func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x45, 0x30, 0, D_80144952);
    record = &table.entry[index & 0xFF];
    func_8014FC90((s16)(x + 0x27 - record->labelLen * 4), (s16)(y + 0x28), 0,
                  record->labelLen, record->label);
    switch (index) {
    case 0:
        count = 1;
        break;
    case 1:
    case 2:
    case 3:
        count = 2;
        break;
    default:
        count = 3;
        break;
    }
    for (i = 0; i < 3; i++) {
        D_801D8F18[i].x = (D_801D8B9C[i].x * func_801783C8(angle) -
                           D_801D8B9C[i].y * func_801782FC(angle)) >> 11;
        D_801D8F18[i].y = (D_801D8B9C[i].x * func_801782FC(angle) +
                           D_801D8B9C[i].y * func_801783C8(angle)) >> 12;
    }
    marker = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(marker);
    marker->x0 = x + 0x26 + D_801D8F18[0].x;
    marker->y0 = y + 0x17 + D_801D8F18[0].y;
    marker->x1 = x + 0x26 + D_801D8F18[1].x;
    marker->y1 = y + 0x17 + D_801D8F18[1].y;
    marker->x2 = x + 0x26 + D_801D8F18[2].x;
    marker->y2 = y + 0x17 + D_801D8F18[2].y;
    shade = (D_801D8F18[0].y << 3) - 0x61;
    marker->b0 = 0;
    marker->r0 = marker->g0 = shade;
    shade = (D_801D8F18[1].y << 3) - 0x61;
    marker->b1 = 0;
    marker->r1 = marker->g1 = shade;
    shade = (D_801D8F18[2].y << 3) - 0x61;
    marker->b2 = 0;
    marker->r2 = marker->g2 = shade;
    func_8014E5A0(1, 0x1C);
    bit = 1;
    for (i = 0; i < count; i++) {
        lit = bit & mask;
        if (lit != 0) {
            flags = D_80143E6C;
            if (flags & 8) {
                colours[i] = ((u32)(flags & 6) << 5) + 0x3F;
            } else {
                colours[i] = ((u32)(~flags & 6) << 5) + 0x3F;
            }
        } else {
            colours[i] = 0x70;
        }
        D_801D8F30[i].x = (record->offset[i][0] * func_801783C8(angle) -
                           record->offset[i][1] * func_801782FC(angle)) >> 11;
        D_801D8F30[i].y = (record->offset[i][0] * func_801782FC(angle) +
                           record->offset[i][1] * func_801783C8(angle)) >> 12;
        bit <<= 1;
    }
    for (i = 0; i < (u8)count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (D_801D8F30[j + 1].y < D_801D8F30[j].y) {
                swap = D_801D8F30[j].x;
                D_801D8F30[j].x = D_801D8F30[j + 1].x;
                D_801D8F30[j + 1].x = swap;
                swap = D_801D8F30[j].y;
                D_801D8F30[j].y = D_801D8F30[j + 1].y;
                D_801D8F30[j + 1].y = swap;
                swap = iconId.kind[j];
                iconId.kind[j] = iconId.kind[j + 1];
                iconId.kind[j + 1] = swap;
                swap = channels[j];
                channels[j] = channels[j + 1];
                channels[j + 1] = swap;
            }
        }
    }
    for (i = 0; i < count; i++) {
        emitPanelIconPrimBate00_801D4678(
            (u16)(x + 0x27 + D_801D8F30[i].x - 4),
            (u16)(y + 0x17 + D_801D8F30[i].y - 8), (u8)(iconId.kind[i] + 3), 0x1E,
            (u16)GetClut(0x10, 0x1E0), channels[i]);
    }
    func_801AF390((s16)x, (s16)y, D_801D8898, 0);
    for (i = 0; i < 6; i++) {
        func_801AF390((s16)(x + i * 8), (s16)y, D_801D88BC, 0);
    }
    func_801AF390((s16)x, (s16)y, D_801D88C8, 0);
}
