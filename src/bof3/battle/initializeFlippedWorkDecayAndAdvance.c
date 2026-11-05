#include "base/types.h"

extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind bss */
void func_801DD3CC(s32 arg0);
void func_801E2314(u32 arg0);
s32 func_8019651C(void* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_8015DF18(u16 arg0);
u8 enemyReadyOrHelper1(void);

/* @source 0x801E4FA8
 * @behavior Initializes the mode-transformed work pair +0x18/+0x1C to
 * -0x1000/0, toggles mode bit 1, clears motion +0x0C/+0x10 and loads the
 * +0x09/+0x0A countdowns with 16/8. Runs the class-0 script helper and
 * positional helper with -4/-4/-4/1, sets flag 0x20 and byte +0x5C to 2,
 * clears the +0x5D..+0x5F triple, submits cue 0x102, runs the enemy readiness
 * helper and advances work byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeFlippedWorkDecayAndAdvance(void)
{
    u8* pair_work;
    u8* motion_work;

    pair_work = D_1F800044;
    *(s32*)(pair_work + 0x18) = -0x1000;
    *(s32*)(pair_work + 0x1c) = 0;
    func_801DD3CC((s32)pair_work);
    D_1F800044[8] ^= 2;
    motion_work = D_1F800044;
    *(s32*)(motion_work + 0x0c) = 0;
    *(s32*)(motion_work + 0x10) = 0;
    motion_work[9] = 16;
    D_1F800044[10] = 8;
    func_801E2314(0);
    func_8019651C(D_1F800044, -4, -4, -4, 1);
    D_1F800044[0] |= 0x20;
    D_1F800044[0x5c] = 2;
    D_1F800044[0x5d] = 0;
    D_1F800044[0x5e] = 0;
    D_1F800044[0x5f] = 0;
    func_8015DF18(0x102);
    enemyReadyOrHelper1();
    D_1F800044[2]++;
}
