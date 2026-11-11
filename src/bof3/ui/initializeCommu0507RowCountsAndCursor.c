#include "bof3/bof3.h"

/* Shared commu queue counters in main RAM, outside every overlay payload. The
 * commu00 overlay lifts record their roles: 0x80145E44 counts the slot indices
 * appended to the removal queue at 0x80145E30, 0x80145E5C is the pending queue
 * count, and 0x80145E5D is the pending queue append index advanced once per
 * notification stored at 0x80145E60. */
extern u8 D_80145E44; /* @source 0x80145E44 @kind data */
extern u8 D_80145E5C; /* @source 0x80145E5C @kind data */
extern u8 D_80145E5D; /* @source 0x80145E5D @kind data */

/* Shared overlay step byte in main RAM; every overlay handler advances it by
 * one as its own step completes. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* This overlay's three section row counts followed by the row cursor. The
 * counts sit in the payload bytes at the tail of the data run based at
 * 0x801D1950, which holds word 0 through word 2 of the handler run before the
 * strings and the drawing tables; the four cells are zero on disc and filled at
 * run time. Each byte is addressed by its own symbol, so each store folds its
 * own address. */
extern u8 D_801D19F0; /* @source 0x801D19F0 @kind data */
extern u8 D_801D19F1; /* @source 0x801D19F1 @kind data */
extern u8 D_801D19F2; /* @source 0x801D19F2 @kind data */
extern u8 D_801D19F4; /* @source 0x801D19F4 @kind data */

/* @source 0x801D0C4C
 * @behavior Step 0 of this overlay's handler run at 0x801D1950, which the
 *           leading blob's step dispatcher at 0x801D0C10 indexes with the
 *           shared step byte 0x801F294C and calls with no arguments and no used
 *           result. Turns each of the three shared commu queue counters into the
 *           row count of one list section, and clears the section row cursor.
 *           Section 0 holds 7 entries per row and takes its counter from
 *           0x80145E44, section 1 holds 0x14 and takes 0x80145E5C, section 2
 *           holds 8 and takes 0x80145E5D; each row count is
 *           ceil(counter / entriesPerRow), stored as (count - 1) / N + 1 while
 *           the counter is set and as 0 while it is clear. The three counts land
 *           in the payload bytes at 0x801D19F0, 0x801D19F1 and 0x801D19F2, the
 *           cursor byte at 0x801D19F4 is cleared, and the step byte at
 *           0x801F294C advances by one. Step 1 of the same run walks the three
 *           counts with that cursor to select the section and the row to draw,
 *           and the three row routines it calls loop 7, 0x14 and 8 entries,
 *           which is where the same three capacities appear.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeCommu0507RowCountsAndCursor(void) {
    u8 count;
    u8 step;

    count = D_80145E44;
    if (count != 0) {
        D_801D19F0 = (count - 1) / 7 + 1;
    } else {
        D_801D19F0 = 0;
    }

    count = D_80145E5C;
    if (count != 0) {
        D_801D19F1 = (count - 1) / 20 + 1;
    } else {
        D_801D19F1 = 0;
    }

    count = D_80145E5D;
    if (count != 0) {
        D_801D19F2 = (count - 1) / 8 + 1;
    } else {
        D_801D19F2 = 0;
    }

    step = D_801F294C;
    D_801D19F4 = 0;
    D_801F294C = step + 1;
}
