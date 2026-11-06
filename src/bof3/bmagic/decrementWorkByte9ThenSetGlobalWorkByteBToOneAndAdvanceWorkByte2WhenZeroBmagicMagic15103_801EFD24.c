#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. Kept as the named cell
 * symbol the exact siblings on this target use; the symbolic address folds into
 * each load (lui $vN,0x1F80 / lw $vN,68($vN)), so the original's two separate
 * reads each re-materialise the base. The plain cell is kept rather than the
 * volatile one several siblings use: no call separates the reads here, and the
 * sibling seedMotionChannelTripletTintAndCountdownThenAdvanceWorkByte2
 * BmagicMagic15103_801EFE90 measured the volatile spelling identical on this
 * same cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work pointer in main RAM, outside this overlay's image. The
 * exact siblings incrementWorkByte2WhenGlobalWorkByte1Is2
 * BmagicMagic15103_801EFD80 and setGlobalWorkByteBTo2ThenInvokeHelper
 * WhenWorkByteBZeroBmagicMagic15103_801EFDC0 read the same cell, and this
 * target's generated undefined_syms_auto.txt already binds the address, so no
 * map row or WEAK_SYMBOL_AT binding is needed. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EFD24
 * @behavior Countdown step of the overlay work object: reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once and decrements the
 *           object's byte at offset 0x9 with a byte-width read-modify-write
 *           (lbu / addiu -1 / sb), then tests the truncated byte it just
 *           stored (andi 0xFF of the stored register, no second load). Only
 *           when that byte reached zero does it store 1 into the byte at
 *           offset 0xB of the shared battle work record pointed to by
 *           0x801EB4E0 - the constant is materialised in the branch delay
 *           slot of the zero test - re-read the pointer cell and increment the
 *           work-object state byte at offset 0x2 (lbu / addiu 1 / sb),
 *           advancing the overlay work byte 2 handler sequence. The injected 1
 *           is the same value the sibling setGlobalWorkByteBTo2
 *           ThenInvokeHelperWhenWorkByteBZeroBmagicMagic15103_801EFDC0 writes
 *           into the same byte. Leaf with no frame; takes no arguments, returns
 *           nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSetGlobalWorkByteBToOneAndAdvanceWorkByte2WhenZeroBmagicMagic15103_801EFD24(void) {
    u8 *work;
    u8 countdown;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        D_801EB4E0[0xB] = 1;
        g_battle_work[2]++;
    }
}
