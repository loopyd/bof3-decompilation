#include "bof3/bof3.h"

extern u8 D_801D1774; /* @source 0x801D1774 @kind data */
extern u8 D_80144952; /* @source 0x80144952 @kind data */
extern u8 D_80148333; /* @source 0x80148333 @kind data */
extern u16 D_80148334; /* @source 0x80148334 @kind data */
extern u16 D_80148336; /* @source 0x80148336 @kind data */
extern u8 D_8014833A; /* @source 0x8014833A @kind data */
extern u8 D_8014833B; /* @source 0x8014833B @kind data */
extern u8 D_8014833C; /* @source 0x8014833C @kind data */
extern u16 D_80148340; /* @source 0x80148340 @kind data */
extern s16 D_80148342; /* @source 0x80148342 @kind data */
extern u8 D_80148354; /* @source 0x80148354 @kind data */
extern u16 D_80148358; /* @source 0x80148358 @kind data */
extern u16 D_8014835A; /* @source 0x8014835A @kind data */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */
extern u16 D_80145AC2; /* @source 0x80145AC2 @kind data */
extern u16 D_80145AC4; /* @source 0x80145AC4 @kind data */
extern u8 *D_801C893C[]; /* @source 0x801C893C @kind data */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */
extern u8 D_801455CA[]; /* @source 0x801455CA @kind data */
extern u8 D_801455CB[]; /* @source 0x801455CB @kind data */
extern u8 D_801F294D; /* @source 0x801F294D @kind data */

extern void func_801AE3F0(u16 x, u16 y, u32 width, u32 height, u32 shade,
                          u32 clut); /* @source 0x801AE3F0 */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height,
                          s32 arg4); /* @source 0x801AED54 */
extern s32 func_8014E640(u32 mask); /* @source 0x8014E640 */
extern void func_8015DF18(u16 cue); /* @source 0x8015DF18 */
extern u16 func_801666C8(u32 bank, u32 entry); /* @source 0x801666C8 */
extern u32 func_801502D0(u32 id); /* @source 0x801502D0 */
extern void func_8014F800(s32 x, s32 y, s32 arg2, s32 arg3,
                          u32 image); /* @source 0x8014F800 */

/* @source 0x801D0D4C
 * @behavior Decrements the nonzero panel step byte, paints the sliding commu
 *           panel, and, while the UI mode is zero, updates a four-bank cursor
 *           over 128 entries in nine-entry pages. Publishes the cursor marker,
 *           plays direction cues, and waits for scrolling to finish before
 *           handling cancel or confirm. Cancel clears the marker, arms UI mode
 *           1 and advances the overlay step. Confirm rejects zero entries;
 *           otherwise it stores the selected entry and bank in the eight-byte
 *           slot selected by D_801448ED and sets overlay step 4. A nonzero
 *           highlighted entry is converted by func_801666C8 and func_801502D0
 *           and drawn at (0x1D, 0x15). Handler word 2 of the run at 0x801D1758;
 *           takes no arguments and returns no value.
 * @status partial
 * @match 79.12
 * @residual Cursor base and start-value allocation differ; origin accesses
 *           rematerialize the fixed address instead of using cursor minus one.
 *           The paged-right arithmetic also reassociates. Best measured shape
 *           has 235 matching instructions and is 20 bytes longer than original.
 */
void drawCommuPanelAndSelectBankEntry(void) {
    s32 direction;
    u8 start;
    u8 column;
    u8 value;
    u8 *cursor;

    if (D_801D1774 != 0) {
        D_801D1774 = D_801D1774 - 1;
    }
    func_801AE3F0(0x14, (0x12 - D_801D1774 * 12) & 0xFFFE,
                  0x118, 0x13, 0xF2, D_80144952);
    func_801AED54(0x16, 0x14 - D_801D1774 * 12, 0x113, 0xF, 0);
    if (D_80148333 != 0) {
        return;
    }
    D_80148354 = 1;
    D_80148358 = D_80148334 + 7;
    D_8014835A = D_80148336 + ((D_8014833C - D_8014833B) * 13 + 0x1A);
    direction = func_8014E640(D_80145AA8 & 0xF00C);
    if (direction & 0x8000) {
        func_8015DF18(0x101);
        D_80148340 = 0x32;
        D_8014833A = D_8014833A - 1;
        if ((s8)D_8014833A < 0) {
            D_8014833A = 3;
        }
    } else if (direction & 0x2000) {
        D_80148340 = 0x31;
        func_8015DF18(0x101);
        D_8014833A = D_8014833A + 1;
        if (D_8014833A >= 4) {
            D_8014833A = 0;
        }
    }
    {
    u8 *pageCursor = &D_8014833C;
    start = *pageCursor;
    if (direction & 0x1000) {
        if (start != 0) {
            *pageCursor = start - 1;
        }
        if (*pageCursor < D_8014833B) {
            D_80148342 = 0xF0;
        }
    } else if (direction & 0x4000) {
        if (start < 0x7F) {
            *pageCursor = start + 1;
        }
        if (*pageCursor >= D_8014833B + 9) {
            D_80148342 = 0x10;
        }
    } else if (direction & 0x4) {
        column = FIELD_REF(u8, pageCursor, -1);
        if (column == 0) {
            *pageCursor = 0;
        } else if (column < 9) {
            *pageCursor = start - column;
            FIELD_REF(u8, pageCursor, -1) = 0;
        } else {
            *pageCursor = start - 9;
            FIELD_REF(u8, pageCursor, -1) = column - 9;
        }
    } else if (direction & 0x8) {
        column = FIELD_REF(u8, pageCursor, -1);
        if (column == 0x77) {
            *pageCursor = 0x7F;
        } else if (column >= 0x6F) {
            *pageCursor = (start + 0x77) - column;
            FIELD_REF(u8, pageCursor, -1) = 0x77;
        } else {
            *pageCursor = start + 9;
            FIELD_REF(u8, pageCursor, -1) = column + 9;
        }
    }
    }
    cursor = &D_8014833C;
    if (start != *cursor) {
        func_8015DF18(0x100);
    }
    if (D_80148342 != 0) {
        return;
    }
    if (D_80145AA8 & D_80145AC4) {
        D_80148354 = 0;
        D_80148333 = 1;
        func_8015DF18(0x106);
        D_801F294D = D_801F294D + 1;
    } else if (D_80145AA8 & D_80145AC2) {
        value = D_801C893C[D_8014833A][*cursor];
        if (value == 0) {
            func_8015DF18(0x107);
            return;
        }
        func_8015DF18(0x105);
        D_801455CA[D_801448ED * 8] = value;
        D_801455CB[D_801448ED * 8] = D_8014833A;
        D_80148333 = 1;
        D_80148354 = 0;
        D_801F294D = 4;
    }
    value = D_801C893C[D_8014833A][D_8014833C];
    if (value != 0) {
        func_8014F800(0x1D, 0x15, 0, 0xFF,
                      func_801502D0(func_801666C8(D_8014833A, value)));
    }
}
