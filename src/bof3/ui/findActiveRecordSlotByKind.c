#include "bof3/bof3.h"

/* Byte 0 of the shared stride-8 active-record slots in main RAM; nonzero marks a
 * slot in use. The 60 slots cover 0x801455C8..0x801457A7. */
extern u8 D_801455C8[]; /* @source 0x801455C8 @kind data */

/* Byte 1 of the same slots: the record kind that this overlay's commu list row
 * at 0x801448ED is matched against, as struct byte +1 of the shared record
 * region. */
extern u8 D_801455C9[]; /* @source 0x801455C9 @kind data */

/* @source 0x801D3AA0
 * @behavior Returns the slot index in the 60 stride-8 active records at
 * 0x801455C8 of the ordinal-th (zero-based) record that is in use (byte 0
 * nonzero) and whose kind byte at 0x801455C9 equals kind; returns 0xFF when
 * fewer than ordinal + 1 records match. No record is modified. The handler at
 * 0x801D0EBC passes the shared commu list row at 0x801448ED as kind and the
 * overlay counter byte at 0x801D4E24 as ordinal, and hands the returned slot to
 * drawRecordSlotChannelBox.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findActiveRecordSlotByKind(u8 kind, u8 ordinal) {
    s32 matches;
    s32 index;

    matches = 0;
    for (index = 0; index < 0x3C; index++) {
        if (D_801455C8[index << 3] != 0) {
            if (D_801455C9[index << 3] == kind) {
                if ((u8)matches == ordinal) {
                    return (u8)index;
                }
                matches++;
            }
        }
    }
    return 0xFF;
}
