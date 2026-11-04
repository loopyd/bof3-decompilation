#include "bof3/bof3.h"

extern u8 T_801EF2A8[128][0x3C]; /* @source 0x801EF2A8 @kind data */

/* @source 0x801EEFB8
 * @behavior Returns the first record with a zero active byte in the overlay's
 *           128-record pool at 0x801EF2A8, or null when all records are active.
 *           Records have a 0x3C-byte stride. The caller at 0x801EF258 passes
 *           the returned record to 0x801EF020, which sets its active byte to 1.
 *           Takes no arguments and does not modify the pool.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 *findFreeRecordBmagicMagic11603_801EEFB8(void) {
    u8 *record;
    u8 index;

    record = T_801EF2A8[0];
    for (index = 0; index < 128; index++, record += 0x3C) {
        if (record[0] == 0) {
            return record;
        }
    }
    return 0;
}
