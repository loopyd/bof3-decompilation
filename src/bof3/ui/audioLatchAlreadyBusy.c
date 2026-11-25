#include "bof3/bof3.h"

/* Action starter of the main executable: runs the action the id names. */
extern void func_80150224(s32 action_id); /* @source 0x80150224 */

/* Frontend mode byte of the main executable; 2 marks the frontend busy. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Byte 0 of this overlay's step cell group: it indexes the five-entry handler
 * run at 0x801DADEC through callStepByte0Table. */
extern u8 D_801F294C; /* @source 0x801F294C @kind data */

/* @source 0x801D5D98
 * @behavior Step 0 handler of the five-entry run at 0x801DADEC. It reads the
 *           shared frontend mode byte at 0x80143BB0 and returns while it
 *           equals 2 (busy). Otherwise it starts action 0xEE through the
 *           main-executable action starter func_80150224, latches the
 *           frontend mode byte to 2, and advances byte 0 of the step group at
 *           0x801F294C by one so the byte-0 dispatcher runs the next handler
 *           of the same run. The step cell is read once, after the action
 *           call and before the mode store, and written back with its own
 *           incremented value. The mode cell is reached through one pointer
 *           local taken before the guard, so its address stays in $s0 across
 *           the action call and both its read and its latch store use that
 *           saved base instead of a fresh %hi/%lo pair. Nothing is returned
 *           and no other cell is read or written. Only word 0 of the
 *           0x801DADEC run names it, so the dispatcher calls it with no
 *           arguments and no used result; no jal in the payload reaches it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void audioLatchAlreadyBusy(void) {
    u8 *mode;
    u8 step;

    mode = &D_80143BB0;
    if (*mode == 2) {
        return;
    }
    func_80150224(0xEE);
    step = D_801F294C;
    *mode = 2;
    D_801F294C = step + 1;
}
