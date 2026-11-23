#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/* Overlay entrypoints this installer publishes. The first refreshes the three
 * party work records and then seeds the battle mode tuple, the second is the
 * battle-exit routine, and the third the mode dispatcher, which reads a byte
 * index in $a0, so it is stored through the void slot type. */
void refreshActivePartyRecords(
    void);
void raiseFlagEnterFrontend(void);
u32 accumulateCountersFlagsHook(u8 mode);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02513Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02513Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02513Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* Work counters of this overlay. All three are read and rewritten only by the
 * mode dispatcher accumulateCountersFlagsHook
 * published here: 0x801FE4F0 accumulates the signed halfword D_80145FAC of the
 * record that dispatcher selects, 0x801FE4E8 accumulates its own operand and
 * 0x801FE4EF is a byte counter of the same dispatch. */
extern u32 D_801FE4F0; /* @source 0x801FE4F0 @kind unknown */
extern u32 D_801FE4E8; /* @source 0x801FE4E8 @kind unknown */
extern u8 D_801FE4EF; /* @source 0x801FE4EF @kind unknown */

/* @source 0x800C2710
 * @behavior Publishes this overlay's handler trio into the shared hook slots:
 * the record-refresh hook
 * refreshActivePartyRecords
 * (refreshes the three 0x140-stride party work records at 0x80145E90, clears
 * each active battler's action flags through 0x801DD14C and starts that
 * record's 0x800F0800 region, then stores 4 into the scenario progress byte
 * and runs initModeTuple530) into 0x801463A4, the battle-exit routine
 * raiseFlagEnterFrontend (raises bit 0x2 of
 * D_801462E8 and requests frontend mode 4) into 0x801463A8, and the mode
 * dispatcher accumulateCountersFlagsHook
 * (six-entry jump table at 0x800C181C) into
 * 0x801463AC, then clears the three work counters that dispatcher maintains:
 * the word 0x801FE4F0, the word 0x801FE4E8 and the byte 0x801FE4EF. It takes no
 * arguments, keeps no frame and touches $v0/$at only, so nothing is returned.
 * The three zero stores each materialise the 0x801F page in $at on their own
 * and appear in source order, the 4F0 word first even though the 4E8 word
 * already covers the byte at 4EF. No in-payload caller: neither this address
 * nor any of the three published addresses appears as a word of this payload,
 * so the main program reaches it through its overlay entry point. Same trio
 * shape as the sibling installer publishBattleHooksBossBoss02513_800C2B60
 * (0x800C2B60), which writes the same three slots in the same order and differs
 * in the published addresses and in having no counter clear; that sibling is
 * shape evidence only, not a clone, since the two bodies differ in both the
 * stored addresses and the trailing stores.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksCounters(void) {
  D_801463A4 =
      refreshActivePartyRecords;
  D_801463A8 = raiseFlagEnterFrontend;
  D_801463AC = (Boss02513Handler)accumulateCountersFlagsHook;
  D_801FE4F0 = 0;
  D_801FE4E8 = 0;
  D_801FE4EF = 0;
}
