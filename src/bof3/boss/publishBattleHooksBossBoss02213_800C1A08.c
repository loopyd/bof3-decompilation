#include "bof3/bof3.h"

typedef void (*Boss02213Handler)(void);

/* Overlay handlers this installer publishes. The first is the battle-entry
 * hook at 0x800C1A48, which seeds the scenario progress byte and the battle
 * mode tuple. The second is the battle setup hook at 0x800C1ABC, which clears
 * one owner record flag, publishes the located owner record through the
 * scratchpad work-record cursor and requests entry 0xAA and resource 6. The
 * third returns a zero result, which the void slot type drops. */
void setupBattleEntryBoss(void);
void setupBattleWorkRecordBossBoss02213_800C1ABC(void);
s32 zeroResultBossBoss02213_800C1A40(void);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02213Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02213Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02213Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1A08
 * @behavior Publishes this overlay handler trio into the shared hook slots in
 * slot order: the battle-entry hook setupBattleEntryBoss
 * into 0x801463A4, the battle setup hook
 * setupBattleWorkRecordBossBoss02213_800C1ABC into 0x801463A8, and the
 * zero-result accessor zeroResultBossBoss02213_800C1A40 into 0x801463AC, the
 * last one through the void slot type. No word of this payload names the
 * function and no jal reaches it, so the main program enters it through the
 * overlay entry point, as it does the sibling installers of this archive
 * family, which write the same three slots in the same order. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02213_800C1A08(void) {
  D_801463A4 = setupBattleEntryBoss;
  D_801463A8 = setupBattleWorkRecordBossBoss02213_800C1ABC;
  D_801463AC = (Boss02213Handler)zeroResultBossBoss02213_800C1A40;
}
