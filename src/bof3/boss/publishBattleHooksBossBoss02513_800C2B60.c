#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/* Overlay entrypoints this installer publishes. The first two are void
 * routines; the third is the overlay's hook-mode dispatcher, which reads a byte
 * mode in $a0 and returns zero, so it is stored through the void slot type. */
void func_800C2D7C(void);
void func_800C2F74(void);
u32 raiseBattleFlagsOrStartBattleWorkByHookModeBossBoss02513_800C2B98(u8 mode);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02513Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02513Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02513Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C2B60
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the progress switch func_800C2D7C (tests bit 0x1 of D_80145E90 and
 * D_80145FD0, calls func_801DD14C, publishes the tested record into the
 * scratchpad work cell 0x1F800044 and starts the 0x800F0800 region through
 * func_8014D5F0) into 0x801463A4, the setup routine func_800C2F74 (starts the
 * 0x801EB630 and 0x801EB748 segments through func_800AC870 and flushes each
 * with func_800AC844) into 0x801463A8, and the hook-mode dispatcher
 * raiseBattleFlagsOrStartBattleWorkByHookModeBossBoss02513_800C2B98
 * (six-entry jump table at 0x800C1834) into 0x801463AC; the dispatcher's mode
 * argument is dropped by the void slot type. No in-payload caller: neither this
 * function's address nor the addresses it stores appear in any word of this
 * payload, so the main program reaches it through its overlay entry point, as
 * it does the sibling installers 0x800C1F9C and 0x800C2088 of
 * BIN/BOSS/BOSS008.EMI#16, 0x800C2114 and 0x800C23F8 of BIN/BOSS/BOSS018.EMI#16
 * and 0x800C24A0 of BIN/BOSS/BOSS055.EMI#16, which write the same three slots
 * in the same order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02513_800C2B60(void) {
  D_801463A4 = func_800C2D7C;
  D_801463A8 = func_800C2F74;
  D_801463AC = (Boss02513Handler)raiseBattleFlagsOrStartBattleWorkByHookModeBossBoss02513_800C2B98;
}
