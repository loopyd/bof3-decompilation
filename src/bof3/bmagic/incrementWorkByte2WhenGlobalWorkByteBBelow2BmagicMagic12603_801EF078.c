#include "bof3/bof3.h"

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* @source 0x801EF078
 * @behavior Reads the shared battle work pointer at 0x801EB4E0 once and
 *           compares its byte at offset 0xB against two with an unsigned bound
 *           (lbu/sltiu), so the guard is taken for every value below two. Only
 *           on that path it reads the scratchpad work pointer cell at
 *           0x1F800044 once and increments the byte at offset 2 of that work
 *           object modulo 256 with lbu/addiu/sb, sharing the loaded base for
 *           the read and the store. Takes no arguments, returns nothing and
 *           keeps no frame, so the scratchpad cell is dereferenced only when
 *           the guard succeeds. Word 5 of the pointer run based at 0x801F125C
 *           names this address (0x801F1270), so it runs as a battle work-step
 *           handler and the payload holds no jal caller for it. Bytes
 *           0x801EF078..0x801EF0BB compared equal by payload cmp to the exact
 *           lift 0x801EF410 of bmagic/magic167/03, whose source supplied the
 *           statement shape; the body is copied here with the scratchpad cell
 *           spelled by this target's own evidenced name and with only the
 *           symbol and file name changed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte2WhenGlobalWorkByteBBelow2BmagicMagic12603_801EF078(void) {
    if (D_801EB4E0[0xB] < 2u) {
        D_1F800044[2]++;
    }
}
