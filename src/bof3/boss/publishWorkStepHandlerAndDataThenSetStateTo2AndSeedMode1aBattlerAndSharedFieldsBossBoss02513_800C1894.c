#include "bof3/bof3.h"

typedef void (*Boss02513StepHandler)(u8 index);

/* The two data pointers this publisher installs both point into this payload's
 * 0x800C3174 data row, the reviewed Splat `textbin` row that fills the last
 * 0x148 bytes of the segment. 0x800C3174 is that row's own label and
 * 0x800C318C is the word at its +0x18, so the second is referenced as its own
 * data symbol rather than as a row start. */
extern u8 D_800C3174; /* @source 0x800C3174 @kind data */
extern u8 D_800C318C; /* @source 0x800C318C @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Battle mode byte the main program publishes at 0x801462EA; this archive
 * family's other step handlers test it against the mode values they serve. */
extern u8 D_801462EA; /* @source 0x801462EA @kind data */

/* The shared halfword run at 0x80144E00, 0x80144E04 and 0x80144E06: the
 * scenario overlay BIN/SCENARIO/SCENA06.EMI#00 seeds those three fields
 * together with their +0x20 mirrors (seedSharedFieldsScenarioScena0600_801FD29C),
 * and this handler copies the run into the work record, so the same shared cell
 * names are used here. */
extern u16 sharedFieldHalfword_80144E00; /* @source 0x80144E00 @kind data */
extern u16 sharedFieldHalfword_80144E04; /* @source 0x80144E04 @kind data */
extern u16 sharedFieldHalfword_80144E06; /* @source 0x80144E06 @kind data */

/* This payload's byte-indexed overlay dispatcher at 0x800C1A68: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C31E4
 * handler table it selects. */
void dispatchIndexedHandlerBossBoss02513_800C1A68(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1E08, 0x800C1D1C and 0x800C1EF4 of
 * BIN/BOSS/BOSS008.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* Shared main-program battle helpers: func_800AC704 returns the index of the
 * first of eight class records whose leading byte matches the id it is given
 * (0xFF when none does), and func_800AC218 copies that class's eight-byte
 * template into the record of the battler index it is given. */
u8 func_800AC704(u8 id);
void func_800AC218(u8 index, u8 class_id);

/* @source 0x800C1894
 * @behavior Work-state entry setup of this overlay, and entry 0 of the run at
 *           0x800C319C (its first word is this function's address) that the
 *           sibling dispatcher 0x800C1850 selects by work-record byte 2:
 *           publishes three words into the current enemy work record loaded
 *           from 0x801EB4E8 -- the data row pointer 0x800C3174 into +0xEC, the
 *           indexed dispatcher dispatchIndexedHandlerBossBoss02513_800C1A68
 *           into +0xE4 and the in-row pointer 0x800C318C (0x800C3174 + 0x18)
 *           into +0xE8 -- then stores 2 into the state byte at +1 of the work
 *           record the main program publishes at the scratchpad pointer cell
 *           0x1F800044 and calls the shared main-program helper func_8014D978.
 *           Writing 2 leaves entry 0 of that state run for the handler the new
 *           record word selects. When the battle mode byte 0x801462EA holds
 *           0x1A it also gives battler 0 the class template of the class record
 *           whose leading byte is 0x59 (func_800AC704 selects that record and
 *           func_800AC218 applies its template), stores 1 into work byte +0x7F
 *           and 0x1E into work byte +0xF0, and copies the shared halfword run
 *           0x80144E00/0x80144E04/0x80144E06 into the record's +0x94 and into
 *           the two runs at +0xA0/+0xA4/+0xA6 and +0xC0/+0xC4/+0xC6. The three
 *           published stores share one load of 0x801EB4E8 while the two guarded
 *           byte stores and the guarded halfword group each reload it, in
 *           source order. The 0x18-byte frame only keeps $ra at 0x10 across the
 *           calls; it takes no arguments and returns nothing. No `jal` names
 *           this address and the sole in-image reference is the 0x800C319C
 *           table word, so the overlay reaches this setup through that
 *           dispatcher. Same publisher shape as the sibling 0x800C1DCC of this
 *           overlay, which installs a different row and dispatcher pair and
 *           carries neither the mode test nor the guarded seeding.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2AndSeedMode1aBattlerAndSharedFieldsBossBoss02513_800C1894(
    void) {
  u8 *work;

  work = D_801EB4E8;
  FIELD_REF(void *, work, 0xECu) = &D_800C3174;
  FIELD_REF(Boss02513StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss02513_800C1A68;
  FIELD_REF(void *, work, 0xE8u) = &D_800C318C;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
  if (D_801462EA == 0x1Au) {
    func_800AC218(0, func_800AC704(0x59u));
    D_801EB4E8[0x7F] = 1;
    D_801EB4E8[0xF0] = 0x1E;
    work = D_801EB4E8;
    FIELD_REF(u16, work, 0x94u) = sharedFieldHalfword_80144E00;
    FIELD_REF(u16, work, 0xC0u) = sharedFieldHalfword_80144E00;
    FIELD_REF(u16, work, 0xA0u) = sharedFieldHalfword_80144E00;
    FIELD_REF(u16, work, 0xC4u) = sharedFieldHalfword_80144E04;
    FIELD_REF(u16, work, 0xA4u) = sharedFieldHalfword_80144E04;
    FIELD_REF(u16, work, 0xC6u) = sharedFieldHalfword_80144E06;
    FIELD_REF(u16, work, 0xA6u) = sharedFieldHalfword_80144E06;
  }
}
