#include "bof3/bof3.h"

/* Shared battle work record pointer in main RAM, outside this overlay's image.
 * This target's generated undefined-syms map already binds the address because
 * the overlay's remaining assembly loads it, so the name needs no map row. The
 * sibling lifts of this target read the same record through this pointer:
 * seedWorkMotionFieldsWhenGlobalWorkByteBIs1BmagicMagic01500_801EFD7C guards on
 * its byte 0xB and decrementGlobalWorkByteBAndInvokeHelperBmagicMagic01500
 * _801F0300 decrements it. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell. Declared as a named symbol so every
 * access is a lui plus offset load, which is how the original re-materializes
 * the cell address before each access group that follows a store through the
 * object. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared readiness helper, outside this overlay's image. Splat binds the
 * address through this target's generated undefined_funcs_auto.txt, so no map
 * row is needed. It reports a readiness byte, which the exact sibling
 * advanceWorkByte2WhenHelperReadyBmagicMagic00800_801F117C tests as a non-zero
 * flag after the same andi 0xFF zero-extension. */
u8 func_8014DAEC(void);

/* @source 0x801F0A94
 * @behavior Per-frame step of the overlay work object, the word the tabledata
 *           segment stores at 0x801EEC7C, entry 30 of the 39-word code-pointer
 *           run that starts at 0x801EEC04. It reads the scratchpad work-object
 *           pointer cell at 0x1F800044 once for the fade guard and the first
 *           store, then re-reads the cell before each of the two remaining
 *           tint stores, because a store through the pointer the cell supplied
 *           may alias it: while the signed tint channel at 0x5D has not reached
 *           its -0x80 floor it subtracts 0x10 from each of the three colour
 *           channels at 0x5D, 0x5E and 0x5F. It then reads byte 0xB of the
 *           shared battle work record read through 0x801EB4E0, and only when
 *           that unsigned byte equals 1, a fresh cell read shows the signed
 *           channel at 0x5D exactly -0x80, and the shared readiness helper
 *           0x8014DAEC returns a non-zero byte, does it copy words 0x34, 0x38
 *           and 0x3C of that record into the work object and advance the work
 *           object's handler byte 2 with a byte-width add (lbu / addiu 1 / sb).
 *           The record pointer is read once more for the copy, so the helper
 *           call is not asked to preserve it. Takes no arguments and returns
 *           nothing; the 0x20-byte frame keeps $ra at 0x18 across the one call.
 *           The 248-byte body is byte-identical to the span at 0x801F1088 of
 *           this same payload - entry 36, the word at 0x801EEC94 - proven by a
 *           payload cmp of the two spans, and that twin is itself still an
 *           unclaimed asm row, so it offered no source to clone. No jal word in
 *           the payload targets 0x801F0A94 and the run's only in-payload reader,
 *           dispatchWorkByte1HandlerBmagicMagic01500_801EECA0, copies its first
 *           four words, so the caller that selects this entry lies outside the
 *           overlay. The nearest lifted sibling,
 *           emi/bmagic/magic001/00@0x801EF5CC
 *           (subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2
 *           WhenChannelIs80), shows the same fade, copy and advance shape at a
 *           leaf with no record guard and no helper call, and supplied the
 *           statement order, the cell re-read points and the stored byte widths
 *           written here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void subtractWorkColorTripletBy16ThenCopyRecordTripletAndAdvanceWorkByte2WhenGlobalWorkByteBIs1ChannelIs80AndHelperReadyBmagicMagic01500_801F0A94(void) {
  u8 *work;
  s32 *source;
  s32 *target;

  work = D_1F800044;
  if (((s8 *)work)[0x5D] != -0x80) {
    ((s8 *)work)[0x5D] -= 0x10;
    D_1F800044[0x5E] -= 0x10;
    D_1F800044[0x5F] -= 0x10;
  }

  if (D_801EB4E0[0xB] == 1 && ((s8 *)D_1F800044)[0x5D] == -0x80 &&
      func_8014DAEC() != 0u) {
    source = (s32 *)D_801EB4E0;
    target = (s32 *)D_1F800044;
    target[13] = source[13];
    target[14] = source[14];
    target[15] = source[15];
    ((u8 *)target)[2]++;
  }
}
