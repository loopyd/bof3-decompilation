#include "bof3/bof3.h"

/* Finds the first record whose active byte is zero in this overlay's
 * 128-record, 0x3C-byte-stride pool at 0x801EF2A8 and returns it, or null once
 * every record is active. Already lifted exact in this target as
 * src/bof3/bmagic/findFreeRecordBmagicMagic11603_801EEFB8.c. */
u8 *findFreeRecordBmagicMagic11603_801EEFB8(void);

/* Initialises one record of the same pool: sets its active byte to 1, byte 1 to
 * 0, byte 2 to 0x10, and seeds its position, scale, flip and countdown fields
 * from the scratchpad work record at 0x1F800044 and rand(). Still an asm span of
 * this target (0x801EF020), so this lift only needs the call-site prototype: one
 * pointer argument in $a0 and no result used. */
void func_801EF020(u8 *record);

/* @source 0x801EF258
 * @behavior Spawn step of this overlay, called once per frame by 0x801EEDB0
 *           before that routine advances the scratchpad work record. It makes up
 *           to eight passes; each pass asks 0x801EEFB8 for the first record of
 *           the 0x801EF2A8 pool whose active byte is zero and, when a record
 *           comes back, initialises that record through 0x801EF020. A pass that
 *           finds no free record initialises nothing and the loop continues. The
 *           counter is a byte, so the original adds one, masks to 0xFF and
 *           compares the masked value unsigned against 8 - the loop runs exactly
 *           eight passes and activates at most eight records. Reads and writes no
 *           other state and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void activateUpTo8FreeRecordsBmagicMagic11603_801EF258(void) {
    u8 *record;
    u8 index;

    for (index = 0; index < 8; index++) {
        record = findFreeRecordBmagicMagic11603_801EEFB8();
        if (record != 0) {
            func_801EF020(record);
        }
    }
}
