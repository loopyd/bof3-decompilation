#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * The overlay's remaining assembly loads the address, so the generated
 * undefined-syms map already binds it and the name needs no map row. The
 * sibling lift subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWork
 * Byte2WhenGlobalWorkByteBIs1ChannelIs80AndHelperReadyBmagicMagic01500_801F1088
 * read word 0x3C of that record to seed the work object in the same way. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Battle record pointer in main RAM, outside this overlay's image, held in the
 * word at 0x8014637C. This overlay's assembly loads that word, so the generated
 * undefined-syms map binds the address and no map row is needed. The bmagic
 * lifts clearRecordFlag40AndInvokeHelperWhenWorkByteBZeroThenAdvanceWorkByte1
 * BmagicMagic00303_801EF1FC and decrementWorkByte9ThenCopyWorkWordTripletSet
 * Byte9To4AndAdvanceWorkByte2OnZeroBmagicMagic07903_801EFE88 read the same cell
 * as a record pointer. */
extern u8 *D_8014637C; /* @source 0x8014637C @kind unknown */

/* Global flag word of the main executable, outside this overlay's image. This
 * overlay's assembly loads it, so the generated undefined-syms map binds the
 * address and no map row is needed. The bmagic lift decrementWorkByte9ByTwo
 * IncrementWorkByteAOnFrameParityThenDecrementGlobalWorkByteBAndClearWorkBytes
 * 0To4AtOneBmagicMagic09203_801EFA24 tests the same word as a one-bit frame
 * parity flag, and this body tests bit 0 exactly as that lift does. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell before each access group that follows a store through the object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper of the main executable, outside this overlay's image.
 * Splat binds the address through this target's generated undefined_funcs_auto
 * file, so no map row is needed. The sibling lifts of this target declare the
 * same prototype and read the byte it returns; this body calls it for its side
 * effect and discards that byte. */
u8 func_8014DAEC(void);

/* Shared main-executable helper, outside every payload. Its role is not
 * established, so the raw address name stays and no map row is needed. This
 * lift passes the record pointer read from 0x8014637C, a 0x2000 value and a
 * zero third argument, the same three argument roles the sibling callAF4A0
 * With3800DecrementWorkByte9AddWorkWord20ToWord14AccumulateIntoHalfword3E
 * CountdownByteAThenAdvanceWorkByte2BmagicMagic01500_801F0DB4 passes with its
 * own literal. */
extern void func_800AF4A0(u8 *record, s32 value, s32 extra); /* @source 0x800AF4A0 */

/* @source 0x801F0F2C
 * @behavior Per-frame step of the overlay work object, the word the tabledata
 *           segment stores at 0x801EEC90: entry 35 of the code-pointer run that
 *           starts at 0x801EEC04, and the fourth of the seven-word work-byte-2
 *           handler run at 0x801EEC84 (0x801F0D48, 0x801F0DB4, 0x801F0E54,
 *           0x801F0F2C, 0x801F1088, 0x801F1180, 0x801F1230) that the step
 *           dispatcher dispatchWorkByte2HandlerThenInvokeSharedServiceAndRunSub
 *           StepBelow4AndPopMatrixWhenWorkByte9SetBmagicMagic01500_801F0C30
 *           copies into its own 0x38-byte frame and selects from with work byte
 *           2 scaled by four. It first calls the shared readiness helper
 *           0x8014DAEC and discards the byte it returns, then passes the battle
 *           record pointer read from the word 0x8014637C together with 0x2000
 *           and a zero third argument to helper 0x800AF4A0. It then reads the
 *           scratchpad work object through the pointer cell 0x1F800044 once and
 *           adds work word 0x20 to work word 0x14, accumulates the updated word
 *           0x14 into work halfword 0x3E, and, while work word 0x14 reads
 *           negative, raises work byte 9 by one per frame but not past 0x14;
 *           when that word reads non-negative it tests bit 0 of the global flag
 *           word 0x80143E6C and, when the bit is set, lowers work byte 9 by one
 *           instead. It then re-reads the cell and, when work word 0x14 still
 *           reads negative and work word 0x3C is below word 0x3C of the record
 *           read through 0x8014638C, raises bit 0x20 of work byte 0, sets work
 *           byte 0x5C to 1, clears the channel bytes 0x5D, 0x5E and 0x5F, copies
 *           the record word 0x3C and advances work byte 2, the handler index.
 *           The cell is re-read before each store group because a store through
 *           the loaded pointer may alias the cell; the guard and the first store
 *           share one cell read, each channel store takes its own, and the last
 *           group keeps one cell read for the word copy and the byte 2 advance,
 *           the group shape the same-target sibling 0x801F1088 measured exact.
 *           The 0x18-byte frame only keeps the return address at 0x10 across the
 *           two calls; the cell at 0x1F800044 and the record pointer are read
 *           after the calls because a call may change them. The body takes no
 *           arguments, returns nothing, and reads or writes no other state. The
 *           roles of work byte 9, work word 0x14, work word 0x20, work word
 *           0x3C, work halfword 0x3E, the flag bit 0x20 of work byte 0 and the
 *           channel bytes 0x5C to 0x5F are not established, so they stay
 *           offset-anchored. The 0x15C-byte span ends exactly at the next
 *           subsegment, 0x801F1088, so the row claims no trailing bytes. No jal
 *           word in the payload targets 0x801F0F2C and the only word naming the
 *           address is the handler-table entry at 0x801EEC90, so the caller that
 *           selects it lies outside the overlay. The 0x15C bytes are not byte
 *           identical to any span of the 396 payloads under out/binaries, which
 *           one payload scan compared.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callReadyHelperAndAF4A0With2000AddWord20ToWord14AccumulateHalfword3ECountByte9BySignOnNegativeWord14BelowRecordWord3CRaiseFlag20SetByte5CToOneClearTripletCopyWord3CAdvanceWorkByte2BmagicMagic01500_801F0F2C(void) {
  u8 *work;
  u8 *object;
  s32 *source;
  s32 *target;

  func_8014DAEC();
  func_800AF4A0(D_8014637C, 0x2000, 0);

  work = D_1F800044;
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  if (*(s32 *)(work + 0x14) < 0) {
    if (work[9] < 0x14) {
      work[9]++;
    }
  } else if (D_80143E6C & 1u) {
    work[9]--;
  }

  object = D_1F800044;
  if (*(s32 *)(object + 0x14) < 0) {
    if (*(s32 *)(object + 0x3C) < *(s32 *)(D_8014638C + 0x3C)) {
      object[0] |= 0x20;
      D_1F800044[0x5C] = 1;
      D_1F800044[0x5D] = 0;
      D_1F800044[0x5E] = 0;
      D_1F800044[0x5F] = 0;
      source = (s32 *)D_8014638C;
      target = (s32 *)D_1F800044;
      target[15] = source[15];
      ((u8 *)target)[2]++;
    }
  }
}
