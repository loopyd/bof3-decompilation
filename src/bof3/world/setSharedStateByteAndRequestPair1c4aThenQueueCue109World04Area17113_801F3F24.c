#include "bof3/bof3.h"

/* Shared overlay state byte of the main executable, outside this overlay's
 * payload. The byte-equal world02/area077/14 handlers at 0x801F5A58 and
 * 0x801F5A84, both lifted exact, write it as the overlay state before queueing a
 * cue, and the world04/area167/13, world04/area196/13, world03/area148/13 and
 * world02/area077/14 target maps all bind this address under this spelling; this
 * target's generated undefined-syms map binds it too, so no row is added here. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Shared request helper of the main executable, outside this overlay's payload
 * and reached only through this body. Its role is unresolved: the exact lifts
 * that call it treat it as a two-value request helper, so the parameters stay
 * structural. This target's generated undefined-funcs map binds the address, so
 * no map row is added. */
extern void func_8015507C(s32 arg0, s32 arg1); /* @source 0x8015507C */

/* Sound-cue dispatcher of the shared main executable, outside this overlay's
 * payload, called with the fixed cue 0x109; the exact neighbour at 0x801F3000
 * calls the same address. This target's generated undefined-funcs map binds it,
 * so no map row is added. */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801F3F24
 * @behavior Overlay handler with no caller in this payload: no jal and no table
 *           word of this overlay names it, so the overlay loader enters it. It
 *           sets the shared overlay state byte D_8014933B to 1, passes the pair
 *           0x1C and 0x4A to the shared request helper func_8015507C, and then
 *           queues the fixed sound cue 0x109 through the shared cue dispatcher
 *           func_8015DF18. The 0x18-byte frame holds only $ra at 0x10 across the
 *           two calls, and the delayed `addiu` that loads each call's first
 *           argument fills the following jal slot. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedStateByteAndRequestPair1c4aThenQueueCue109World04Area17113_801F3F24(
    void) {
  D_8014933B = 1;
  func_8015507C(0x1C, 0x4A);
  func_8015DF18(0x109);
}
