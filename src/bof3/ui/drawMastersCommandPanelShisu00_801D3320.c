#include "bof3/bof3.h"

/* The panel task root the overlay entry at 0x801D324C hands this body. Its
 * prefix repeats the shared PanelTask layout (the state byte the entry
 * dispatches on and the two origin halfwords) because the shared header stops
 * at +0x08, and the two fields only this drawer reads follow it. The pointer at
 * +0x20 is read as one byte per command row; what those bytes mean beyond that
 * is unproven, so the name stays address anchored.
 */
typedef struct ShisuCommandPanelTask {
    u8 unk_00[3];
    u8 unk_03;         /* +0x03 state byte the caller dispatches on */
    u16 x;             /* +0x04 panel origin column */
    u16 y;             /* +0x06 panel origin row */
    u8 unk_08[2];
    u8 selected;       /* +0x0A index of the selected command row */
    u8 unk_0B[0x15];
    const u8 *unk_20;  /* +0x20 one state byte per command row */
} ShisuCommandPanelTask;

/* Shared CLUT-bank byte of the main executable that every SHISU panel passes as
 * the trailing argument of its inset painter.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* The four command codes of this payload's initialized blob, one per command
 * row, read at the row index. The overlay entry at 0x801D1CA4 maps the same
 * four codes through func_8016628C into its UI work block head, so the run is
 * the row-order code table rather than four unrelated bytes.
 * @source 0x801D9798 @kind table */
extern const u8 D_801D9798[];

/* The panel title of the same blob: the glyph run "SHISU", measured with
 * strlen to right-align it inside the panel.
 * @source 0x801D979C @kind data */
extern const char D_801D979C[];

/* The two 0xC-byte sprite-record streams of the same blob, each submitted at
 * the panel origin with the flag 1. */
extern const u8 D_801D9780[]; /* @source 0x801D9780 @kind data */
extern const u8 D_801D978C[]; /* @source 0x801D978C @kind data */

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. This payload only calls
 * it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Command-row emitter of the main executable: the two signed coordinates, the
 * shade, the third argument this body always passes as zero, the row's command
 * code, the row's state byte and the row flag. Only the positions this call
 * site fills are evidenced, so every role stays positional.
 * @source 0x801AF800 */
extern void func_801AF800(s16 arg0, s16 arg1, s32 arg2, s32 arg3, u8 arg4,
                          u8 arg5, s32 arg6);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries. This payload only
 * calls it.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8 *arg2, u8 arg3);

/* Sprite emitter of the main executable: it fills one GT quad from the
 * sprite-record table entry the id selects and appends it at slot 1. This
 * payload only calls it.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* Text drawer of the main executable: the row position, the shade, the glyph
 * width and the glyph-code run. This payload only calls it.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* PsyQ libc string length, called only for its answer here. */
extern int strlen(const char *arg0);

/* @source 0x801D3320
 * @behavior Draws the whole SHISU panel of the masters system-mode overlay from
 *           its panel task root: the 0x95 by 0xA0 inset at (x + 3, y + 3) with
 *           the shade 0 and the shared CLUT-bank byte, then the four command
 *           rows at x + 7 and y + 0x18 or 0x1A in steps of 13, each carrying
 *           its own code byte of D_801D9798 and its own state byte of the
 *           task's +0x20 run. A row whose state byte is zero is emitted with
 *           the shade 7 and the row flag 1, and a row whose state byte is not
 *           zero with the shade 0 and the flag 0. The selected row, the index
 *           the task's +0x0A byte names, is emitted at y+0x18 and, when its
 *           own state byte is not zero, gains one extra emission at y+0x1A
 *           with the shade 7 and the flag 1. The title D_801D979C is then
 *           right-aligned through strlen into the panel and drawn at
 *           (x + 0x4E, y + 7) with the shade 0 and the glyph width 0x10, the
 *           two sprite streams D_801D9780 and D_801D978C are submitted at the
 *           panel origin with the flag 1, and the panel is closed by the border
 *           pieces 1 along the top from x + 40 to x + 112, 4 down the left side
 *           from y + 24 to y + 152, 5 at (x, y + 0xA0), 0x20 at (x + 0x90,
 *           y + 0x18), 0x21 down the right side from y + 32 to y + 144, 0x22 at
 *           (x + 0x90, y + 0x98), 6 along the bottom from x + 8 to x + 136, and
 *           the two top corner pieces 0x32 at (x + 8, y) and 0x34 at
 *           (x + 0x90, y), all with the flag 1. The payload names this body
 *           only from the jal at 0x801D3288 inside func_801D324C, which reads
 *           the panel task root from the shared cell 0x80148648 after
 *           dispatching its own state byte, so that root is the only argument
 *           its call site shows. Its 0x38-byte frame keeps $s0 through $s4 and
 *           $ra, and the epilogue returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMastersCommandPanelShisu00_801D3320(ShisuCommandPanelTask *task) {
    u8 i;

    func_801AE3F0((u16)(task->x + 3), (u16)(task->y + 3), 0x95, 0xA0, 0,
                  D_80144952);
    i = 0;
    do {
        u8 shade;
        u8 flag;

        if (task->unk_20[i] != 0) {
            shade = 0;
            flag = 0;
        } else {
            shade = 7;
            flag = 1;
        }
        if (i != task->selected) {
            func_801AF800((s16)(task->x + 7),
                          (s16)((u32)task->y + (i * 13 + 0x1A)), shade, 0,
                          D_801D9798[i], task->unk_20[i], flag);
        } else {
            if (task->unk_20[i] != 0) {
                func_801AF800((s16)(task->x + 7),
                              (s16)((u32)task->y + (i * 13 + 0x1A)), 7, 0,
                              D_801D9798[i], task->unk_20[i], 1);
            }
            func_801AF800((s16)(task->x + 7), (s16)(task->y + i * 13 + 0x18),
                          shade, 0, D_801D9798[i], task->unk_20[i], flag);
        }
        i++;
    } while (i < 4);

    func_8014F800((s16)(task->x + 0x4E - strlen(D_801D979C) * 4),
                  (s16)(task->y + 7), 0, 0x10, (u32)D_801D979C);
    func_801AF390((s16)task->x, (s16)task->y, D_801D9780, 1);
    func_801AF390((s16)task->x, (s16)task->y, D_801D978C, 1);

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
        func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + (i + 4) * 8),
                      0x21, 1);
        i++;
    } while (i < 0xF);
    func_801AF2A0((s16)task->x, (s16)(task->y + 0xA0), 5, 1);
    func_801AF2A0((s16)(task->x + 0x90), (s16)(task->y + 0x98), 0x22, 1);
    func_801AF2A0((s16)(task->x + 8), (s16)task->y, 0x32, 1);
    func_801AF2A0((s16)(task->x + 0x90), (s16)task->y, 0x34, 1);
    i = 0;
    do {
        func_801AF2A0((s16)(task->x + (i + 1) * 8), (s16)(task->y + 0xA0), 6,
                      1);
        i++;
    } while (i < 0x11);
}
