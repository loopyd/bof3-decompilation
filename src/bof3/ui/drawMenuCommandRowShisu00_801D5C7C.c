#include "bof3/bof3.h"

/* The system-menu command-code table: five-byte records whose first byte is the
 * number of commands of that group and whose following bytes are command codes
 * into the label table below. The five records hold four, four, four, two and
 * two codes, so the byte after the last code of each record is unused slack.
 * Only this body reads the table and it sits in the payload's final
 * initialized blob, so the address keeps its generated name.
 */
extern u8 D_801D9CE8[][5]; /* @source 0x801D9CE8 @kind table */

/* The command-label table: one string pointer per command code, from "Use" at
 * index 0 through "End" at index 0x16, over the label strings of the same
 * initialized blob. The body indexes it with the code byte of the record above
 * and hands the pointer to the width query and to the text drawer.
 */
extern const char *D_801D9C8C[]; /* @source 0x801D9C8C @kind table */

/* The two 0x18-byte sprite-record streams emitted once per cell: the second is
 * the one the selected entry takes, the first is the one every other entry
 * takes. Both lie in the same initialized blob and are only ever submitted,
 * never modified.
 */
extern const u8 D_801D9904[]; /* @source 0x801D9904 @kind unknown */
extern const u8 D_801D991C[]; /* @source 0x801D991C @kind unknown */

/* The shared main-RAM CLUT-bank byte the panel emitter takes as its last
 * argument. It lies below this overlay's load address, in the main executable,
 * so this target only reads it.
 */
extern u8 D_80144952; /* @source 0x80144952 @kind data */

/* The shared main-RAM flag byte whose bit 0 clears the highlight of command
 * code 0x12 in this body. It lies in the main executable as well.
 */
extern u8 D_80143F02; /* @source 0x80143F02 @kind data */

/* The main-executable helpers this body calls. Their widths are the ones this
 * call site shows, not recovered callee contracts: the two queries are declared
 * word-wide and narrowed here, and the panel emitter's last argument is the
 * zero-extended byte load of the CLUT-bank cell.
 */
void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                   u32 arg5);
void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);
u32 func_801AF20C(const void *arg0);
void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);
u32 func_80166140(u32 arg0);
s32 func_8016628C(s32 table, s32 code, s32 mode);

/* @source 0x801D5C7C
 * @behavior Draws one row of system-menu command cells: the group argument
 *           selects a five-byte record of D_801D9CE8 whose first byte is the
 *           command count and whose following bytes are command codes, and for
 *           every code the body emits the shaded 0x2D by 0x14 panel at the x
 *           argument plus 0x30 per entry and the row y through func_801AE3F0
 *           with the shared CLUT-bank byte, sets the shade byte 7 for each
 *           entry other than the selection while the selection lies inside the
 *           count, forces that shade for the Ally code 0x12 while bit 0 of
 *           D_80143F02 is clear and for the Chng code 8 while func_80166140
 *           with 0xF and func_8016628C with 0, 0x58 and 0 both answer zero,
 *           loads the command label D_801D9C8C[code] once, submits it through
 *           the text drawer func_8014F800 at x plus the 0x30 step plus a
 *           right-aligned four-cell inset and y plus 3 with width 0x10, and
 *           emits the 0x18-byte sprite-record stream D_801D991C for the
 *           selected entry or D_801D9904 for any other entry at x plus the
 *           0x30 step and the sign-extended y. The count byte is re-read from
 *           D_801D9CE8 at each test because the calls can reach it, the
 *           0x50-byte frame keeps $ra and the eight saved registers, and the
 *           callers' fifth stack word is never read here.
 * @status partial
 * @match 93.15
 * @residual Both in-loop count loads use the original symbol-plus-index
 *           address form and the body has the original 0x248-byte size.
 *           The record pointer and selection use s6 and s5 respectively,
 *           rather than s5 and s6, and their preheader initialization order
 *           differs. Three record-index additions reverse their commutative
 *           operands, and the text inset's addiu by 6 follows the x addition
 *           rather than preceding it. The shop span at 0x801DA77C differs
 *           only in seven words binding four payload-local data addresses,
 *           each shifted by the same 0xBCD8 delta.
 */
void drawMenuCommandRowShisu00_801D5C7C(u16 x, u16 y, u8 group, u8 selected)
{
    const char *label;
    u32 offset;
    u8 shade;
    u8 i;

    offset = group * 5;
    i = 0;
    if (((u8 *)D_801D9CE8)[offset] == 0) {
        return;
    }
    do {
        u8 *record;

        record = &((u8 *)D_801D9CE8)[offset];
        func_801AE3F0((u16)(x + i * 0x30), (u16)y, 0x2D, 0x14, 0, D_80144952);
        shade = 0;
        if (selected < D_801D9CE8[group][0]) {
            shade = (i != selected) ? 7 : 0;
        }
        if (record[i + 1] == 0x12) {
            if ((D_80143F02 & 1) == 0) {
                shade = 7;
            }
        }
        if (record[i + 1] == 8) {
            if ((u8)func_80166140(0xF) == 0) {
                if ((u16)func_8016628C(0, 0x58, 0) == 0) {
                    shade = 7;
                }
            }
        }
        label = D_801D9C8C[record[i + 1]];
        func_8014F800((s16)(x + i * 0x30 +
                           (4 - (u8)func_801AF20C(label)) * 4 + 6),
                      (s16)(y + 3), shade, 0x10, (u32)label);
        func_801AF390((s16)(x + i * 0x30), (s16)y,
                      (i != selected) ? D_801D9904 : D_801D991C, 0);
        i++;
    } while (i < D_801D9CE8[group][0]);
}
