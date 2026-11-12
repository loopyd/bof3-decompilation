#include "bof3/bof3.h"

/* Shared helper in the main executable that consumes the unsigned battle byte
 * at 0x80146384. It lives outside this overlay's image, so it keeps its raw
 * address name and the binding this target's generated undefined_funcs_auto.txt
 * supplies. */
void func_800A9820(u8 arg0);

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays.
 * The cell is read once per access, so the original re-materializes its address
 * (lui $vN,0x1F80 / lw $vN,68($vN)) for each access. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Global battle bytes in the main executable, outside this overlay's image.
 * Each is read once with lbu and zero-extended. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */
extern u8 D_801462EC; /* @source 0x801462EC @kind unknown */

/* @source 0x801EFF48
 * @behavior Entry index 10 (the word at 0x801F1BEC) of the twelve-entry code
 *           pointer run at 0x801F1BC4 that
 *           dispatchWorkByte1HandlerBmagicMagic13103_801EF668 indexes with the
 *           scratchpad work object's byte 1, so it runs as that step handler
 *           while work byte 2 is still zero. Reads the work-object pointer cell
 *           at 0x1F800044 once and decrements work byte 9 at byte width (lbu /
 *           addiu -1 / sb); the byte the guard compares is the stored register
 *           truncated to eight bits (andi / bnez), so no second cell read
 *           supplies the compared value. Only when that countdown reaches zero
 *           does it forward the global battle byte 0x80146384 to the
 *           main-executable helper 0x800A9820 and then read the global mode
 *           byte 0x801462EC: mode 0 or 3 seeds the work word pairs 0xC/0x10 and
 *           0x18/0x1C with 8/-1/1/-1, and any other mode seeds them with
 *           -8/-1/-1/-1. It then re-reads the pointer cell for each of the
 *           remaining three stores: work byte 9 is cleared, work byte 0xA is
 *           set to 0x1E, and work byte 2 is advanced with a byte-width
 *           read-modify-write (lbu / addiu 1 / sb), which is the store in the
 *           return path. The pointer-cell load precedes frame creation and the
 *           0x18-byte frame only keeps $ra at 0x10 across the helper call.
 *           Takes no arguments, returns nothing and reads or writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenForwardBattleByteSeedMotionPairByModeSetByteATo1EAndAdvanceWorkByte2BmagicMagic13103_801EFF48(void) {
    u8 *work;
    u8 countdown;
    u8 mode;
    u32 *pair;
    u32 *other;

    work = g_battle_work;
    countdown = work[9] - 1;
    work[9] = countdown;
    if (countdown == 0) {
        func_800A9820(D_80146384);
        mode = D_801462EC;
        if (mode == 0 || mode == 3) {
            pair = (u32 *)g_battle_work;
            pair[3] = 8;
            pair[4] = -1;
            pair[6] = 1;
            pair[7] = -1;
        } else {
            other = (u32 *)g_battle_work;
            other[3] = -8;
            other[4] = -1;
            other[6] = -1;
            other[7] = -1;
        }
        g_battle_work[9] = 0;
        g_battle_work[0xA] = 0x1E;
        g_battle_work[2]++;
    }
}
