#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 before its frame store, then
 * again for the countdown tail and once more for each of the two stores of the
 * reload arm, so the cell is read through this target's map name rather than
 * through a constant-address accessor, whose shared base would stay live across
 * the call in a saved register and widen the frame. */
extern u8* scratchWorkPointerAt1; /* @source 0x1F800044 @kind data */

/* Overlay draw helper: keeps its first argument in $s1 and hands that block to
 * the transform calls at 0x801AFF04 and 0x801AFFD8, stores its second argument
 * into the two stack halfwords at 0x2A and 0x28 with sh, and splits its third
 * argument with andi 0x4, andi 0x2 and andi 0x1 into the shifts 5, 6 and 7, so
 * its three roles are block base, signed halfword and channel mask rather than
 * a count. */
void func_801F46EC(u8* block, s16 value, s32 channel_mask);

/* @source 0x801F3910
 * @behavior Overlay progress step handler, entry 2 of the handler row based at
 *           0x801F5F88: it hands the draw helper 0x801F46EC the work record
 *           block at +0x64, the signed halfword at +0x2E and the channel mask
 *           6, then decrements the countdown byte at +0x09 as an unsigned byte
 *           (the store wraps at 0xFF, and the andi 0xFF after it is the
 *           truncation of the stored value) and returns while the decremented
 *           count is nonzero. On the frame that count reaches zero it re-arms
 *           that byte with 0x20, which the original materialises in the branch
 *           delay slot of the countdown test, and increments the state byte at
 *           +0x01, handing the record to entry 3 (0x801F3994) of the row. The
 *           record pointer comes from the scratchpad cell 0x1F800044 and is
 *           never an argument: the function takes none, returns nothing, and
 *           its 0x18-byte frame exists only to keep $ra at 0x10 across the
 *           helper call. All four cell accesses are separate loads because the
 *           byte store through the loaded pointer may alias the cell.
 *
 *           Dispatch evidence: the payload word at 0x801F5F90 is 0x801F3910
 *           (cmp of the payload word against this address), the third word of
 *           the handler row based at 0x801F5F88 (0x801F3814, 0x801F3878,
 *           0x801F3910, 0x801F3994, 0x801F3AA8, ...), and the exact dispatcher
 *           dispatchStateByte1Table5f88World02Area09813_801F37D0 scales the
 *           work record byte at +0x01 by four onto that base and jalrs the
 *           selected entry with no arguments, so this span is the state-2
 *           handler of that row and its caller stays outside the payload. No
 *           jal immediate names this address, so the absence of arguments is
 *           structural rather than declared.
 *
 *           Naming evidence: entry 1 of the same row, the still-anchored
 *           0x801F3878, calls the same helper with the same three arguments
 *           (block +0x64, the signed halfword +0x2E re-read through the cell,
 *           mask 6) and carries the same countdown tail, differing only in that
 *           it advances +0x2E by 0x20 before the call and re-arms +0x09 with
 *           0x80 plus cue 0x209; entry 0, the exact
 *           stateWorkWords64Advance, seeds the
 *           three words at +0x64 with the shared words at 0x80145EC4 to
 *           0x80145ECC, sets +0x09 to 8, clears +0x2E and advances +0x01, so the
 *           countdown this body ticks is the one that seed arms and the block it
 *           draws is that three-word seed. The helper's argument roles are read
 *           off 0x801F46EC itself, which keeps $a0 as a base for the transform
 *           calls and splits $a2 with andi 0x4, andi 0x2 and andi 0x1, so the
 *           third argument is a channel mask.
 *
 *           Shape evidence: the 0x84-byte span is not byte-identical to any
 *           span of the corpus - a scan of every payload under out/binaries for
 *           the whole span found only this copy, so no clone applied. Two
 *           siblings are equal to it in every word except the jal immediate at
 *           span offset 0x18 (a payload cmp showed exactly those two differing
 *           bytes): 0x801D0D54 in emi/scenario/sce15ef1/00 and 0x801F3400 in
 *           emi/world04/area188/13. Both differ only in which local helper they
 *           call, so they are statement-shape seeds and not clone proofs; the
 *           sce15ef1 copy is already lifted exact
 *           (invokeOriginHelperThenTickCountdownReload32AndAdvanceWorkByte1ScenarioSce15ef100_801D0D54)
 *           and supplied the statement order, operand forms and declaration
 *           style of this body, with this target's own literals, offsets and
 *           callee taken from the bytes above.
 *
 *           Representation notes: the countdown is tested on the promoted u8
 *           decrement, which is the form that carries the andi 0xFF truncation
 *           after the store, and the cell is re-read for the two stores of the
 *           reload arm instead of reusing the tail pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callBlockTickCountdown(void) {
  u8* work;

  work = scratchWorkPointerAt1;
  func_801F46EC(work + 0x64, *(s16*)(work + 0x2E), 6);
  if (--scratchWorkPointerAt1[9] == 0) {
    scratchWorkPointerAt1[9] = 0x20;
    scratchWorkPointerAt1[1]++;
  }
}
