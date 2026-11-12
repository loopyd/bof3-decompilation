#include "bof3/bof3.h"

/* Appends one primitive of the given size class to the ordering table; the
 * helper lives in the main executable, outside this payload. */
extern void func_8014E5A0(u32 ot_index, u32 primitive_size); /* @source 0x8014E5A0 */

/* The shared primitive cursor the engine publishes for the overlay emitters. */
extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

/* Shared frame counter of the main executable; its low bits select the blink
 * tint of an unselected marker. */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Pending world id and the commu entry count of a world, one byte of the table
 * 0x801F286D indexed by that id. */
extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_801F286D[]; /* @source 0x801F286D @kind data */

/* One 12-byte entry of the twelve-entry layout table at 0x801D4C44, indexed by
 * the commu list row byte 0x801448ED: the cell's top-left corner (x, topY) and
 * bottom-right corner (rightX, bottomY), plus the notch pair (notchX, notchY)
 * that the chamfered form of the outline starts its top and left segments at. */
typedef struct CommuRowLayout {
    u16 notchX;
    u16 topY;
    u16 x;
    u16 notchY;
    u16 rightX;
    u16 bottomY;
} CommuRowLayout;

extern CommuRowLayout D_801D4C44[]; /* @source 0x801D4C44 @kind data */

/* @source 0x801D36FC
 * @behavior Emits the outline of one commu01/07 list row cell as two
 *           flat-shaded three-point lines at the shared primitive cursor, one
 *           0x18-byte packet each from g_PrimCursor, stamped through SetLineF3
 *           and appended through func_8014E5A0(1, 0x18). The row byte selects
 *           the entry of the twelve-entry layout table at 0x801D4C44. The
 *           first packet runs from (x, topY) to (rightX, topY) to (rightX,
 *           bottomY) and the second from (x, topY or notchY) to (x, bottomY)
 *           to (rightX, bottomY), so together they draw the cell's top, right
 *           and bottom edges and its left edge below the notch, and the segment
 *           between (x, topY) and (notchX, notchY) is left open. Only a row of
 *           1 through 8 whose entry count (0x801F286D indexed by the pending
 *           world id 0x80143F00) is below the row starts both packets at the
 *           cell corner (x, topY), the full outline; every other row starts the
 *           first packet at (notchX, topY) and the second at (x, notchY), which
 *           chamfers that corner. Both packets carry white 0xFF in the red and
 *           green bytes and zero in blue when the selected byte is nonzero; a
 *           zero selected byte takes bits 1 and 2 of the shared frame counter
 *           0x80143E6C, scaled by 32 and offset by 0x3F, inverted while bit 3
 *           of that counter is clear, so an unselected row outline blinks. The
 *           flat-line packets are the only state written; no value is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRowMarkerOutlineCommu0107_801D36FC(u8 row, u8 selected)
{
    LINE_F3* line;
    u32 color;
    u32 flags;

    if (selected == 0) {
        flags = D_80143E6C;
        if (flags & 8) {
            color = ((flags & 6) << 5) | 0x3F;
        } else {
            color = ((~flags & 6) << 5) | 0x3F;
        }
    } else {
        color = 0xFF;
    }

    line = (LINE_F3*)g_PrimCursor;
    SetLineF3(line);
    if (row != 0 && row < 9 && D_801F286D[D_80143F00] < row) {
        line->x0 = D_801D4C44[row].x;
    } else {
        line->x0 = D_801D4C44[row].notchX;
    }
    line->y0 = D_801D4C44[row].topY;
    line->x1 = D_801D4C44[row].rightX;
    line->y1 = D_801D4C44[row].topY;
    line->x2 = D_801D4C44[row].rightX;
    line->y2 = D_801D4C44[row].bottomY;
    line->r0 = color;
    line->g0 = color;
    line->b0 = 0;
    func_8014E5A0(1, 0x18);

    line = (LINE_F3*)g_PrimCursor;
    SetLineF3(line);
    if (row != 0 && row < 9 && D_801F286D[D_80143F00] < row) {
        line->y0 = D_801D4C44[row].topY;
    } else {
        line->y0 = D_801D4C44[row].notchY;
    }
    line->x0 = D_801D4C44[row].x;
    line->x1 = D_801D4C44[row].x;
    line->y1 = D_801D4C44[row].bottomY;
    line->x2 = D_801D4C44[row].rightX;
    line->y2 = D_801D4C44[row].bottomY;
    line->r0 = color;
    line->g0 = color;
    line->b0 = 0;
    func_8014E5A0(1, 0x18);
}
