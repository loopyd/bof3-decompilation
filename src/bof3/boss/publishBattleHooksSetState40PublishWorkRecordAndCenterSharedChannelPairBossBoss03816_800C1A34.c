#include "bof3/bof3.h"

typedef void (*Boss03816Handler)(void);

/* Overlay entrypoints this installer publishes into the shared battle hook
 * slots. The first is this target's exact battle-flag state switch 0x800C1B10,
 * the second its exact inert handler 0x800C1B74, and the third its exact
 * zero-result accessor 0x800C1B08, which the void slot type drops. */
void initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss03816_800C1B10(
    void);
void noopHandlerBossBoss03816_800C1B74(void);
s32 zeroResultBossBoss03816_800C1B08(void);

/* Scratchpad work-record pointer cell: the battle code publishes a work record
 * here and every access reaches that record through this cell. Declared as a
 * named cell so each access folds to its own lui plus the offset store, which is
 * what the original emits for the store and for the load that follows the call.
 * Splat binds this address through this target's own map row. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Work record this entry publishes into the scratchpad cell. The body reads the
 * record's word pair at 0x34 and 0x38, the same position pair the exact battle
 * lift func_801DF3F8 republishes from D_80143F24/D_80143F28, so the declaration
 * is a byte image and those two words stay offset-anchored. */
extern u8 D_801EBDD8; /* @source 0x801EBDD8 @kind unknown */

/* Battle state byte in main RAM, outside this overlay's image. The boss and
 * bmagic lifts of this tree both read and write it as the state selector they
 * pass to the shared per-state handlers, and they write small constants into it
 * (3, 4, 5, 0x80 and 0xF). The role of this particular 0x40 is not established
 * by this body, so the name reports the write. */
extern u8 D_80146384; /* @source 0x80146384 @kind unknown */

/* Shared work position pair in main RAM, outside this overlay's image. The exact
 * battle lift func_801DF3F8 republishes it into work words 0x34 and 0x38, so it
 * is the position pair this entry averages the published record against. Signed
 * words, because the original divides the difference with the two-instruction
 * sign fix-up of a signed division by two. */
extern s32 D_80143F24; /* @source 0x80143F24 @kind unknown */
extern s32 D_80143F28; /* @source 0x80143F28 @kind unknown */

/* Shared fixed-point channel pair and its scale halfword in main RAM, outside
 * this overlay's image. The bmagic lifts add their motion pair into the same two
 * words, the sibling boss lift that arms the work step reads them as the shared
 * channel base, and the world lifts publish a scale halfword at the same 0x2E
 * offset. Signed words here because this body stores a signed midpoint in
 * them. */
extern s32 D_80149308; /* @source 0x80149308 @kind unknown */
extern s32 D_8014930C; /* @source 0x8014930C @kind unknown */
extern u16 D_8014932E; /* @source 0x8014932E @kind unknown */

/* Shared hook slots. 0x801463A4 and 0x801463A8 are the battle hooks the battle
 * step loads and calls: the battle overlay header include/bof3/battle/
 * battle03_internal.h binds both as handler cells, and its own lifts call the
 * loaded pointers. 0x801463AC is the third slot of the same trio. The exact
 * same-target sibling publishBattleHooksBossBoss03816_800C1B7C writes the same
 * three slots in the same order through the same slot type. */
extern Boss03816Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss03816Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss03816Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* Shared main-executable helper the bmagic and boss lifts call with no
 * arguments and no result; this caller passes nothing either. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */

/* @source 0x800C1A34
 * @behavior Overlay entry that installs this payload's opening battle state. It
 *           first publishes the overlay handler trio into the shared battle hook
 *           slots in slot order: the battle-flag state switch 0x800C1B10 into
 *           0x801463A4, the inert handler 0x800C1B74 into 0x801463A8 and the
 *           zero-result accessor 0x800C1B08 into 0x801463AC, each materialised
 *           in $v0 with lui/addiu and stored through $at. It then writes 0x40 to
 *           the battle state byte 0x80146384, publishes this overlay's work
 *           record 0x801EBDD8 into the scratchpad pointer cell 0x1F800044, saves
 *           $ra at 0x10 of its 0x18-byte frame and runs the shared helper
 *           0x800AFD9C with no arguments.
 *           After that call it re-reads the scratchpad cell and centres the
 *           shared fixed-point channel pair on the midpoint between the record's
 *           position pair and the shared work position: word 0x80149308 takes
 *           0x80143F24 plus half the signed difference to the record word 0x34,
 *           and word 0x8014930C takes 0x80143F28 plus half the signed difference
 *           to the record word 0x38. It also stores 0x20 into the shared scale
 *           halfword 0x8014932E, which the original materialises between the two
 *           midpoints' arithmetic while it fills the load gap of the second
 *           one. The original loads each shared position word once and reuses it
 *           for both the subtraction and the final addition, and it emits the
 *           sign fix-up of a division by two, so the midpoint truncates toward
 *           zero.
 *           No jal in this payload names this span and no word of the payload
 *           holds its address, so the overlay loader reaches it and its
 *           parameter roles stay structural: it takes no arguments, leaves the
 *           last midpoint in $v0 and its frame only keeps $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishBattleHooksSetState40PublishWorkRecordAndCenterSharedChannelPairBossBoss03816_800C1A34(
    void) {
  u8 *work;

  D_801463A4 =
      initModeTuple510ByBattleFlagAndSetScenarioProgressBossBoss03816_800C1B10;
  D_801463A8 = noopHandlerBossBoss03816_800C1B74;
  D_801463AC = (Boss03816Handler)zeroResultBossBoss03816_800C1B08;
  D_80146384 = 0x40;
  D_1F800044 = &D_801EBDD8;
  func_800AFD9C();
  work = D_1F800044;
  D_80149308 = D_80143F24 + (*(s32 *)(work + 0x34) - D_80143F24) / 2;
  D_8014930C = D_80143F28 + (*(s32 *)(work + 0x38) - D_80143F28) / 2;
  D_8014932E = 0x20;
}

/* Statement-order probes measured before the first passing comparison, kept here
 * because the comparison only reports the winner. Writing the 0x20 scale store
 * between the two midpoint statements left 49 of 53 instructions matching, with
 * the constant and its store pulled up into the load gap of the first midpoint;
 * writing it ahead of both left 50 of 54 and grew the span to 216 bytes. Only
 * last, after both midpoints, reproduces the original placement of that store
 * and the 212-byte span. */
