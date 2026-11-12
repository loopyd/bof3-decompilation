#include "bof3/bof3.h"

/* Byte 0 of the shared stride-8 active-record slots in main RAM; nonzero marks a
 * slot in use. The 60 slots cover 0x801455C8..0x801457A7. */
extern u8 D_801455C8[]; /* @source 0x801455C8 @kind data */

/* Byte 1 of the same slots: the record kind counted here, as struct byte +1 of
 * the shared record region. */
extern u8 D_801455C9[]; /* @source 0x801455C9 @kind data */

/* @source 0x801D3A48
 * @behavior Returns how many of the 60 stride-8 active records at 0x801455C8
 * are in use (byte 0 nonzero) and have the kind byte at 0x801455C9 equal to the
 * low byte of kind, truncated to an unsigned byte. No record is modified. Its
 * callers pass the shared commu list row 0x801448ED, the overlay byte
 * 0x801D4E28, a literal 0 and other list rows, and test the result as a bound
 * or against a fixed row count.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 countActiveRecordsByKind(s32 kind) {
    u8 count;
    s32 offset;

    count = 0;
    kind &= 0xFF;
    for (offset = 0; offset < 0x1E0; offset += 8) {
        if (D_801455C8[offset] != 0 && D_801455C9[offset] == kind) {
            count++;
        }
    }
    return count;
}
