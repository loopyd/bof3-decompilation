#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_801462EC; /* @source 0x801462EC @kind unknown */

/* @source 0x801EFA68
 * @behavior Per-frame step of a write byte 0x2 state handler of this overlay:
 * reads the scratchpad work-object pointer cell at 0x1F800044 once and loads
 * the object's countdown byte 0x9. It adds the current work words 0xC and 0x10
 * into work halfwords 0x2E and 0x30, then re-reads words 0xC and 0x10 and adds
 * work words 0x18 and 0x1C into them, keeping the 0x2E halfword store before
 * the word 0xC store and the word 0x10 store in the branch delay slot.
 * When the decremented countdown byte is zero it reads the global mode byte
 * D_801462EC: mode 0 or 3 seeds the word pairs 0xC/0x10 and 0x18/0x1C with
 * 4/4 and -1/-1, any other mode seeds them with -4/4 and 1/-1. It then stores
 * 6 into byte 0x9, re-reads the scratchpad pointer cell and increments state
 * byte 0x2 with a byte-width add (lbu / addiu 1 / sb). Leaf with no frame;
 * takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addMotionPairIntoHalfwordsDecrementCountdownSetByte9To6SeedMotionPairByModeAdvanceWorkByte2BmagicMagic13103_801EFA68(void) {
    u8 *work;
    u32 *words;
    u16 *halves;
    u8 countdown;
    u8 mode;
    u32 *pair;
    u32 *other;
    u8 *reset;
    u8 *step;

    work = g_battle_work;
    words = (u32 *)work;
    halves = (u16 *)work;
    countdown = work[9] - 1;
    halves[0x17] += words[3];
    halves[0x18] += words[4];
    words[3] += words[6];
    words[4] += words[7];
    work[9] = countdown;
    if (countdown == 0) {
        mode = D_801462EC;
        if (mode == 0 || mode == 3) {
            pair = (u32 *)g_battle_work;
            pair[3] = 4;
            pair[4] = 4;
            pair[6] = -1;
            pair[7] = -1;
        } else {
            other = (u32 *)g_battle_work;
            other[3] = -4;
            other[4] = 4;
            other[6] = 1;
            other[7] = -1;
        }
        reset = g_battle_work;
        reset[9] = 6;
        step = g_battle_work;
        step[2]++;
    }
}
