#include "bof3/bof3.h"

typedef void (*Boss02916Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the state switch
 * at 0x800C1930, which tests bit 1 of D_80146328 and either stores 0x1C into
 * D_80146875, calls func_801DDED8 and stores 0x62 into D_80145029, or calls
 * func_801DDF00. The second takes no argument and forwards zero to
 * func_800AC934. The third returns a zero result, which the void slot type
 * drops. */
void func_800C1930(void);
void callOwnerRecordHead(void);
s32 zeroResult18(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02916Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02916Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02916Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C18F0
 * @behavior Publishes this overlay handler trio into the shared hook slots in
 * slot order: the state switch at 0x800C1930 into 0x801463A4, the
 * zero-argument wrapper callOwnerRecordHead into 0x801463A8,
 * and the zero-result accessor zeroResult18 into
 * 0x801463AC, the last one through the void slot type. The 0x38-byte body
 * reserves no frame and keeps nothing across the three stores. No word of this
 * payload names this function and no jal reaches it, so the main program
 * enters it through the overlay entry point, as it does the sibling
 * installers 0x800C1E3C of BIN/BOSS/BOSS002.EMI#16 and 0x800C1D70 of
 * BIN/BOSS/BOSS033.EMI#16, which write the same three slots in the same order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooks34(void) {
  D_801463A4 = func_800C1930;
  D_801463A8 = callOwnerRecordHead;
  D_801463AC = (Boss02916Handler)zeroResult18;
}
