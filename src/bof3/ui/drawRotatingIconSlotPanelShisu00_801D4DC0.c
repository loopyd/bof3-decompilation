#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One (x, y) pair of the panel's working tables, held in the overlay's own RAM
 * blob so the sort below can reorder whole pairs. 0x8 bytes.
 */
typedef struct ShisuIconSlotOffset {
    s32 x; /* +0x00 */
    s32 y; /* +0x04 */
} ShisuIconSlotOffset;

/* One entry of the ten-entry label table the panel indexes with its selector
 * byte: the ASCII label of the entry, the number of cells that label occupies
 * and the (x, y) offset of each of the three icon slots of the entry. The 0x14
 * bytes of the entry are what the panel's record stride multiplies.
 */
typedef struct ShisuIconSlotRecord {
    u8 label[7];      /* +0x00 ASCII cells of the label, not terminated */
    u8 labelLen;      /* +0x07 cells of label the text drawer is given */
    s16 offset[3][2]; /* +0x08 (x, y) offset of each icon slot */
} ShisuIconSlotRecord;

/* The ten label records as one object, so the panel can take them by one
 * assignment. 10 * 0x14 = 0xC8 bytes.
 */
typedef struct ShisuIconSlotTable {
    ShisuIconSlotRecord entry[10]; /* +0x00 */
} ShisuIconSlotTable;

/* The label table of the payload's initialised blob: the ten entries are
 * "Normal", "Normal", "Attack", "Defense", "Normal", "Attack", "Defense",
 * "Chain", "Magic" and "Refuge", and the byte after each label is that label's
 * own length.
 * @source 0x801D0C34 @kind data */
extern ShisuIconSlotTable D_801D0C34;

/* The three icon kinds of the panel's slots, stored as signed bytes in the same
 * blob right after the label table: 0, 1 and 2, so the panel draws the kinds
 * three, four and five of the icon emitter's atlas.
 * @source 0x801D0CFC @kind table */
extern s8 D_801D0CFC[3];

/* The four (x, y) model offsets of the rotating backdrop marker, right after the
 * label blob: (0, 0), (-12, -1), (-4, 12) and (4, 12). The panel rotates the
 * first three into its own scratch table and draws them as one Gouraud
 * triangle.
 * @source 0x801D9BC0 @kind data */
extern ShisuIconSlotOffset D_801D9BC0[];

/* The panel's two three-entry scratch tables in the same RAM blob: the rotated
 * backdrop-marker vertices and the rotated icon-slot offsets.
 * @source 0x801D9EF8 @kind data */
extern ShisuIconSlotOffset D_801D9EF8[3]; /* @source 0x801D9EF8 @kind data */

/* @source 0x801D9F10 @kind data */
extern ShisuIconSlotOffset D_801D9F10[3];

/* The three sprite-record streams the panel submits at its own origin: the
 * backdrop cluster, one six-cell row walked in eight-pixel steps and the closing
 * stream. They lie in the same initialised blob and are only ever submitted.
 * @source 0x801D98BC @kind unknown */
extern const u8 D_801D98BC[];

/* @source 0x801D98E0 @kind unknown */
extern const u8 D_801D98E0[];

/* @source 0x801D98EC @kind unknown */
extern const u8 D_801D98EC[];

/* Shared CLUT-bank byte of the main executable that every SHISU panel passes as
 * the trailing argument of its inset painter.
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

/* Panel icon emitter of this overlay, already lifted in this target. The
 * original zero-extends both icon coordinates, so this call site keeps the
 * unsigned coordinates the emitter's signed halfword fields are filled from.
 * @source 0x801D5620 */
extern void emitPanelIconPrimShisu00_801D5620(u16 arg0, u16 arg1, s32 arg2,
                                             s32 arg3, u16 arg4, u8 arg5);

/* @source 0x801D4DC0
 * @behavior Rotating icon-slot panel of the SHISU overlay. It copies the
 *           payload's ten-entry label table into its frame, loads the three
 *           icon kinds 0, 1 and 2 beside it, paints the 0x45 by 0x30 inset
 *           panel at (x + 3, y + 3) through func_801AE3F0 with the shade 0 and
 *           the shared CLUT-bank byte D_80144952, and takes the record the
 *           selector byte indexes with a 0x14-byte stride. The record's label is
 *           submitted through func_8014FC90 at (x + 0x27 - labelLen * 4,
 *           y + 0x28) as a labelLen-cell run with the alignment 0, so it is
 *           right aligned on the panel. The slot count is 1 for the selector 0,
 *           2 for the selectors 1 through 3 and 3 for every other selector.
 *           One statement later the body rotates the payload's four model
 *           offsets through func_801783C8 and func_801782FC, keeping the first
 *           three as its backdrop-marker vertices in D_801D9EF8 with the
 *           horizontal term shifted down by 11 and the vertical term by 12.
 *           Those vertices fill one Gouraud triangle taken from g_PrimCursor
 *           and stamped by func_8017A97C: the three vertex pairs at (x + 0x26,
 *           y + 0x17) plus the rotated offsets, the red and green bytes of each
 *           vertex set to that vertex's rotated y shifted left by three and
 *           biased by -0x61, the blue bytes zero, and the 0x1C-byte packet appended at
 *           slot 1 through func_8014E5A0. It then fills one colour byte per slot
 *           from the low bits of the colour-enable argument: a set bit takes the
 *           flat colour 0x3F | (((D_80143E6C & 8) ? (D_80143E6C & 6) :
 *           (~D_80143E6C & 6)) << 5) and a clear bit the grey 0x70. In the same
 *           walk it rotates the record's slot offsets through the same two
 *           helpers into D_801D9F10, with the same two shifts, then sorts the
 *           slots by their rotated y with a bubble sort that swaps the rotated
 *           pair, the icon kind and the colour through signed-byte and byte
 *           temporaries. Finally it emits one 16x16 icon per slot through
 *           emitPanelIconPrimShisu00_801D5620 at (x + 0x27 + slot x - 4,
 *           y + 0x17 + slot y - 8) with the icon kind plus 3 as its kind, the
 *           0x1E texture row, the clut GetClut(0x10, 0x1E0) and the slot's
 *           colour byte, and submits the three sprite streams D_801D98BC at
 *           (x, y), D_801D98E0 six times from x in eight-pixel steps and
 *           D_801D98EC at (x, y) through func_801AF390 with the flag 0. No jal
 *           and no table word of this payload names this body, so it is an
 *           overlay entry called from outside the payload; its 0x130-byte frame
 *           keeps $s0 through $s7, $fp and $ra and the epilogue returns no
 *           value. One payload cmp found the 0x74C bytes of
 *           func_801D3E18 at 0x801D3E18 in emi/etc/bate/00 and of
 *           func_801D98C0 at 0x801D98C0 in emi/etc/shop/00 differing in 39 of
 *           their 467 words, and those words are the payload-local data halves,
 *           the jal this payload's own icon emitter takes and the internal j
 *           words, so the three are one body and those two rows, both still
 *           `asm` rows, are shape evidence rather than a clone source.
 * @status partial
 * @match 77.30
 * @residual Both live bodies reserve 0x130 bytes and use the same saved-register
 *           offsets. The first difference saves and loads the phase in $s5
 *           rather than $s7; the palette-byte pointer uses $s7 rather than $s5.
 *           Scratch temporaries also differ. The unsigned selector test emits
 *           sltiu without the original bltz arm, and the three icon kinds use
 *           separate computed addresses rather than one base register. Colour
 *           selection, marker stores and sort accesses also differ in branch
 *           topology, addressing or scheduling. The live body is 1820 bytes
 *           against the original 1868. Three further clean-C lifetime and
 *           selector probes did not improve the retained 361/467 comparison.
 */
void drawRotatingIconSlotPanelShisu00_801D4DC0(u16 x, u16 y, u8 index, u8 mask,
                                               s32 angle)
{
    ShisuIconSlotTable table;
    ShisuIconSlotRecord *record;
    POLY_G3 *marker;
    s8 iconId[3];
    u8 channels[3];
    u8 *colours;
    u8 lit;
    u8 count;
    u32 bit;
    u8 i;
    u8 j;
    s32 flags;
    u32 colour;
    s8 swap;

    table = D_801D0C34;
    iconId[0] = D_801D0CFC[0];
    iconId[1] = D_801D0CFC[1];
    iconId[2] = D_801D0CFC[2];
    func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x45, 0x30, 0, D_80144952);
    record = &table.entry[index & 0xFF];
    func_8014FC90((s16)(x + 0x27 - record->labelLen * 4), (s16)(y + 0x28), 0,
                  record->labelLen, record->label);
    count = 3;
    if (index == 0) {
        count = 1;
    } else if (index < 4) {
        count = 2;
    }
    for (i = 0; i < 3; i++) {
        D_801D9EF8[i].x =
            (D_801D9BC0[i].x * func_801783C8(angle) -
             D_801D9BC0[i].y * func_801782FC(angle)) >> 11;
        D_801D9EF8[i].y =
            (D_801D9BC0[i].x * func_801782FC(angle) +
             D_801D9BC0[i].y * func_801783C8(angle)) >> 12;
    }
    marker = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(marker);
    marker->x0 = x + 0x26 + D_801D9EF8[0].x;
    marker->y0 = y + 0x17 + D_801D9EF8[0].y;
    marker->x1 = x + 0x26 + D_801D9EF8[1].x;
    marker->y1 = y + 0x17 + D_801D9EF8[1].y;
    marker->x2 = x + 0x26 + D_801D9EF8[2].x;
    marker->y2 = y + 0x17 + D_801D9EF8[2].y;
    marker->b0 = 0;
    marker->r0 = marker->g0 = (D_801D9EF8[0].y << 3) - 0x61;
    marker->b1 = 0;
    marker->r1 = marker->g1 = (D_801D9EF8[1].y << 3) - 0x61;
    marker->b2 = 0;
    marker->r2 = marker->g2 = (D_801D9EF8[2].y << 3) - 0x61;
    func_8014E5A0(1, 0x1C);
    bit = 1;
    colours = channels;
    for (i = 0; i < count; i++) {
        lit = bit & mask;
        if (lit != 0) {
            flags = D_80143E6C;
            if (flags & 8) {
                colour = ((u32)(flags & 6) << 5) + 0x3F;
            } else {
                colour = ((u32)(~flags & 6) << 5) + 0x3F;
            }
        } else {
            colour = 0x70;
        }
        colours[i] = colour;
        D_801D9F10[i].x =
            (record->offset[i][0] * func_801783C8(angle) -
             record->offset[i][1] * func_801782FC(angle)) >> 11;
        D_801D9F10[i].y =
            (record->offset[i][0] * func_801782FC(angle) +
             record->offset[i][1] * func_801783C8(angle)) >> 12;
        bit <<= 1;
    }
    for (i = 0; i < (u8)count - 1; i++) {
        for (j = 0; j < count - 1 - i; j++) {
            if (D_801D9F10[j + 1].y < D_801D9F10[j].y) {
                swap = D_801D9F10[j].x;
                D_801D9F10[j].x = D_801D9F10[j + 1].x;
                D_801D9F10[j + 1].x = swap;
                swap = D_801D9F10[j].y;
                D_801D9F10[j].y = D_801D9F10[j + 1].y;
                D_801D9F10[j + 1].y = swap;
                swap = iconId[j];
                iconId[j] = iconId[j + 1];
                iconId[j + 1] = swap;
                swap = colours[j];
                colours[j] = colours[j + 1];
                colours[j + 1] = swap;
            }
        }
    }
    for (i = 0; i < count; i++) {
        emitPanelIconPrimShisu00_801D5620(
            (u16)(x + 0x27 + D_801D9F10[i].x - 4),
            (u16)(y + 0x17 + D_801D9F10[i].y - 8), (u8)(iconId[i] + 3), 0x1E,
            (u16)GetClut(0x10, 0x1E0), channels[i]);
    }
    func_801AF390((s16)x, (s16)y, D_801D98BC, 0);
    for (i = 0; i < 6; i++) {
        func_801AF390((s16)(x + i * 8), (s16)y, D_801D98E0, 0);
    }
    func_801AF390((s16)x, (s16)y, D_801D98EC, 0);
}
