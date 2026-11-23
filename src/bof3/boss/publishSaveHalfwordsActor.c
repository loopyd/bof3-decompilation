#include "bof3/bof3.h"

typedef void (*Boss02513Handler)(void);

/* Overlay entrypoints this installer publishes. The first two are void
 * routines lifted in this target; the third is the overlay's hook-mode
 * dispatcher func_800C2208, which masks a byte mode in $a0, rejects modes above
 * six and jumps through the seven-entry table at 0x800C1804, so it is stored
 * through the void slot type and keeps its raw name until it is lifted. */
void stateRestoreSavedHalfwords(
    void);
void enterFrontendMode4RestoreSavedSlotIdsAndSetRecordCount3BossBoss02513_800C2680(
    void);
void func_800C2208(u8 mode);

/* Shared hook slots. 0x801463A4/0x801463A8 are the battle hooks the battle
 * step loads and calls (jalr); 0x801463AC is the third slot of the same trio,
 * written by every boss overlay of this archive family. */
extern Boss02513Handler D_801463A4; /* @source 0x801463A4 @kind unknown */
extern Boss02513Handler D_801463A8; /* @source 0x801463A8 @kind unknown */
extern Boss02513Handler D_801463AC; /* @source 0x801463AC @kind unknown */

/* The 0x140-stride local work record halfword pair this installer backs up and
 * re-seeds: 0x80145F18 and 0x80145F1A are record offsets 0x88 and 0x8A of the
 * record based at 0x80145E90, and 0x80145F20/0x80145F22 are the pair eight and
 * ten bytes above them that supplies the re-seed. The first handler this
 * installer publishes (0x800C2634) reads the two overlay cells declared below
 * back into 0x80145F18/0x80145F1A. */
extern u16 D_80145F18; /* @source 0x80145F18 @kind unknown */
extern u16 D_80145F1A; /* @source 0x80145F1A @kind unknown */
extern u16 D_80145F20; /* @source 0x80145F20 @kind unknown */
extern u16 D_80145F22; /* @source 0x80145F22 @kind unknown */

/* Overlay-resident bytes the low half of each of those two halfwords is saved
 * into. Both sit in the zero tail of this payload's 0x800C3174 textbin run, so
 * the generated undefined-symbol list of this target carries them and no
 * target-map row is added. Nothing else in the payload reads or writes them
 * except the 0x800C2634 handler that restores the pair. */
extern u8 D_800C32B4; /* @source 0x800C32B4 @kind unknown */
extern u8 D_800C32B8; /* @source 0x800C32B8 @kind unknown */

/* The global actor-phase byte of the main program, read by
 * isBattlerEligibleForActorPhase (0x801DB9E4), which rejects a slot whose side
 * code (2 local, 1 enemy) or 3 already equals it, so this store leaves the
 * phase at the value that selects no side. */
extern u8 D_80146324; /* @source 0x80146324 @kind unknown */

/* @source 0x800C2184
 * @behavior Overlay installer of this archive family: it publishes this
 * overlay's handler trio into the shared battle hook slots in slot order, the
 * step handler
 * stateRestoreSavedHalfwords
 * into 0x801463A4, the front-end handoff handler
 * enterFrontendMode4RestoreSavedSlotIdsAndSetRecordCount3BossBoss02513_800C2680
 * into 0x801463A8 and the hook-mode dispatcher func_800C2208 into 0x801463AC.
 * It then backs up the local work record's halfword pair: it loads the record
 * halfwords 0x80145F18/0x80145F1A and their 0x80145F20/0x80145F22
 * re-seed pair before it writes anything but the slots, stores the low byte of
 * each record halfword into the overlay cells 0x800C32B4 and 0x800C32B8, and
 * re-seeds 0x80145F18/0x80145F1A from the re-seed pair with the step-byte-style
 * store order, one byte backup followed by one halfword re-seed per record
 * offset. It finally stores 3 into the global actor-phase byte 0x80146324, the
 * value isBattlerEligibleForActorPhase treats as selecting no side. The 0x84
 * byte body reserves no frame, keeps nothing across it, touches
 * $v0/$v1/$a0-$a3/$at only and returns nothing. The address 0x80145F18 is
 * materialised once in $v1 and read and written through offset 0 of that base
 * while the other five cells take their own lui/addiu pair, so the two halves
 * of that one cell are the only access this body reaches through a held base.
 * That base is a local pointer to the cell, which is also the representation
 * lever of this lift: with the cell read and written as the extern directly,
 * the compiler hoisted the four halfword loads above the three slot stores and
 * dropped the held base (20/33 instructions, first difference at the first
 * instruction and a re-materialised lui/addiu pair before the re-seed store);
 * holding the cell in a pointer keeps the loads after the slot stores and the
 * one base, which is what this range's bytes show. The constant 3 store sits
 * between the four loads and the four record stores.
 * No word of this payload names this address and no `jal` reaches it, so the
 * main program enters the overlay through it, as it does the sibling installers
 * 0x800C1E50 of BIN/BOSS/BOSS001.EMI#13, 0x800C1F9C and 0x800C2088 of
 * BIN/BOSS/BOSS008.EMI#16 and 0x800C2B60 of this same payload, which write the
 * same three slots in the same order and carry none of the record work.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishSaveHalfwordsActor(
    void) {
  u16* cell;
  u16 record0;
  u16 reseed0;
  u16 record1;
  u16 reseed1;

  D_801463A4 =
      stateRestoreSavedHalfwords;
  D_801463A8 =
      enterFrontendMode4RestoreSavedSlotIdsAndSetRecordCount3BossBoss02513_800C2680;
  D_801463AC = (Boss02513Handler)func_800C2208;
  cell = &D_80145F18;
  record0 = *cell;
  reseed0 = D_80145F20;
  record1 = D_80145F1A;
  reseed1 = D_80145F22;
  D_80146324 = 3;
  D_800C32B4 = record0;
  *cell = reseed0;
  D_800C32B8 = record1;
  D_80145F1A = reseed1;
}
