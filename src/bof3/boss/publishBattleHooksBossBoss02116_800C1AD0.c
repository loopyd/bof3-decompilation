#include "bof3/bof3.h"

typedef void (*Boss02116Handler)(void);

/* Overlay entrypoints this installer publishes. The first is the mode-transition
 * handler: it tests the 0x2 bit of D_80146328, and while it is set stores 0xA
 * into the scenario progress byte and runs the main-program service
 * func_801DDED8, otherwise runs func_801DDF00. The second is the owner-zero
 * battle entry: it publishes the owner-0 work record to the scratchpad work
 * cursor 0x1F800044, clears record byte 0x2A, requests resource 4 and clears
 * the owner-record heads for ids 1 and 2. The third is the index dispatcher: it
 * masks its argument with 0xFF and, while the index is below 6, jumps through
 * the six-word table at 0x800C1804; it returns zero, which the void slot type
 * drops. */
void initModeTupleByBattleFlagAndSetScenarioProgressBossBoss02116_800C1C18(void);
void publishOwnerRecordClearWorkByte2AAndRequestResource4ThenClearOwnerHeads1And2BossBoss02116_800C1C64(
    void);
s32  func_800C1B08(s32 arg0);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02116Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02116Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02116Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* @source 0x800C1AD0
 * @behavior Publishes this overlay handler trio into the shared hook slots: the
 * mode-transition handler
 * initModeTupleByBattleFlagAndSetScenarioProgressBossBoss02116_800C1C18 (tests
 * the 0x2 bit of D_80146328, and while it is set stores 0xA into the scenario
 * progress byte and calls func_801DDED8, otherwise calls func_801DDF00) into
 * 0x801463A4, the owner-zero battle entry
 * publishOwnerRecordClearWorkByte2AAndRequestResource4ThenClearOwnerHeads1And2BossBoss02116_800C1C64
 * (publishes the owner-0 work record to the scratchpad work cursor 0x1F800044,
 * clears record byte 0x2A, requests resource 4, then clears the owner-record
 * heads for ids 1 and 2) into 0x801463A8, and the index dispatcher
 * func_800C1B08 (masks its argument with 0xFF and, while the index is below 6,
 * jumps through the six-word table at 0x800C1804, returning zero) into
 * 0x801463AC; the dispatcher result is dropped by the void slot type. Same trio
 * shape as the sibling installers of this archive family, which write the same
 * three slots in the same order; this body is the 0x38-byte form the family
 * shares and differs from the exact sibling at 0x800C1AB0 of the boss024/16
 * overlay only in the three stored pointers, each of its three addiu low-half
 * words shifted by exactly that pointer address delta. It takes no arguments
 * and returns nothing. No in-payload caller: no other word of this payload
 * holds this function address and no jal reaches it, so the main program
 * reaches it through its overlay entry point.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksBossBoss02116_800C1AD0(void) {
  D_801463A4 =
      initModeTupleByBattleFlagAndSetScenarioProgressBossBoss02116_800C1C18;
  D_801463A8 =
      publishOwnerRecordClearWorkByte2AAndRequestResource4ThenClearOwnerHeads1And2BossBoss02116_800C1C64;
  D_801463AC = (Boss02116Handler)func_800C1B08;
}
