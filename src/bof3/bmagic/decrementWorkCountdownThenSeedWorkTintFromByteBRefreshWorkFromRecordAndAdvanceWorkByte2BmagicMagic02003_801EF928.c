#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt, so
 * no target-map row is needed. The exact lifts of this target read the same
 * record (addWorkColorTripletBy16ThenSetGlobalWorkByteBToFFAndAdvanceWorkByte2
 * WhenChannelIsZeroBmagicMagic02003_801EF7D0 writes its byte 0xB), and its byte
 * 0x27 and words 0x34, 0x38 and 0x3C are the fields this handler copies out. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell, spelled the same named cell the exact
 * lifts of this target already use. A named cell symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell after each store through the pointer it
 * supplied, which a store through that loaded pointer forces. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x801EF928
 * @behavior Entry 0 of the three-entry work-byte-2 handler table based at
 *           0x801EEC3C, whose three words are 0x801EF928 (this body),
 *           0x801EFA70 and 0x801EFAD4; the dispatcher at 0x801EF894 stages that
 *           table on its own frame and indexes it with work-object byte 0x2, so
 *           this body runs while byte 0x2 still holds 0. It decrements the
 *           scratchpad work-object countdown at byte 9 (lbu / addiu -1 / sb) and
 *           tests the truncated byte it just stored (andi 0xFF of the stored
 *           register, no second load of the cell) against zero, returning
 *           through the jr $ra at 0x801EFA68 while the countdown is still
 *           running. Only on the last tick does it run the seeding path: it
 *           re-reads the cell and raises bit 0x20 of work byte 0 (lbu / ori /
 *           sb), sets work byte 0x5C to 1, seeds each of the work tint bytes
 *           0x5D, 0x5E and 0x5F to -(((byte 0xB) + 1) * 20) at byte width,
 *           re-reading the cell and byte 0xB before each channel because a store
 *           through the pointer it supplied may alias the cell, and clears work
 *           byte 0x2B. It then copies byte 0x27 of the shared battle work record
 *           read through 0x801EB4E0 into work byte 0x27, and copies record words
 *           0x34, 0x38 and 0x3C over the matching work words, incrementing work
 *           byte 0x2 at byte width before the last word store lands, which
 *           advances the dispatcher to handler 1 at 0x801EFA70. Leaf with no
 *           arguments, no return value and no stack frame; it reads and writes
 *           no other state. The seeding statements repeat the shape the exact
 *           lift decrementWorkCountdownThenSeedWorkTintFromByteBRefreshWorkFrom
 *           RecordAndAdvanceWorkByte2BmagicMagic01500_801EFFC0 records for the
 *           same handler in a sibling overlay, minus its conditional step size.
 *
 *           A byte search over every payload under out/binaries found exactly
 *           one copy of this 0x148-byte span, so the clone lever was declined.
 *           The span ends in jr $ra and seeds from the pointer it loads, so the
 *           data lever did not apply, and it sits between the reviewed
 *           boundaries at 0x801EF924 (end of 0x801EF894) and 0x801EFA70, so the
 *           carve lever did not apply either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkCountdownThenSeedWorkTintFromByteBRefreshWorkFromRecordAndAdvanceWorkByte2BmagicMagic02003_801EF928(void) {
  u8 *work;
  u8 countdown;
  u8 *object;
  u8 *mode;
  u8 *source;
  u8 *copyByte;
  s32 *record;
  s32 *copy;

  work = D_1F800044;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    object = D_1F800044;
    object[0] |= 0x20;
    mode = D_1F800044;
    mode[0x5C] = 1;
    ((s8 *)D_1F800044)[0x5D] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((s8 *)D_1F800044)[0x5E] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((s8 *)D_1F800044)[0x5F] = -((((u8 *)D_1F800044)[0xB] + 1) * 20);
    ((u8 *)D_1F800044)[0x2B] = 0;
    source = D_801EB4E0;
    copyByte = D_1F800044;
    copyByte[0x27] = source[0x27];
    record = (s32 *)D_801EB4E0;
    copy = (s32 *)D_1F800044;
    copy[13] = record[13];
    copy[14] = record[14];
    copy[15] = record[15];
    ((u8 *)copy)[2] += 1;
  }
}
