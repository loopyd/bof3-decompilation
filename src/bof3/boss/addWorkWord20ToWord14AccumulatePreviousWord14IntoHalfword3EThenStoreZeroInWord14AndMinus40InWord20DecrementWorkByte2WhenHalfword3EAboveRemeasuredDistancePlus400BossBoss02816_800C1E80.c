#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, bound by its own D_ name and not by
 * SPAD_PTR_SLOT: the original materialises the base once (lui $v0,0x1F80 /
 * lw $v0,68($v0)) ahead of its own frame setup and reaches every access of its
 * first statement group through that one register, while the macro's
 * absolute-constant lvalue makes the compiler hold 0x1F800044 in a
 * callee-saved register and load through offset zero. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program helper the whole boss archive family calls: it returns
 * the distance between the fixed-point position pair it takes as (x, z). Both
 * uses here add a slack to the returned word and compare or store the whole
 * word, so the return type is the full `s32` at this site. This target's
 * generated undefined_funcs_auto.txt already binds the address, so the callee
 * needs no map row. */
extern s32 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x800C1E80
 * @behavior Per-frame field integrator and arrival reset of this payload's work
 *           state machine: it loads the work record pointer from the scratchpad
 *           pointer cell 0x1F800044 once, ahead of its own frame setup, and adds
 *           the record's unsigned halfword 0x3E by its word 0x14 as that word
 *           stood before this update, then advances word 0x14 by word 0x20. The
 *           word 0x14 the halfword update consumes is the original's own second
 *           load of that address rather than a copy of the value the update
 *           stores, because the halfword statement precedes the update in the
 *           body and its store to 0x3E may alias the word. It re-reads halfword
 *           0x3E as a signed value, takes the func_8015477C distance of the
 *           record's position word pair at 0x34 and 0x38, and when the
 *           accumulated halfword is still above that distance plus 0x400 it
 *           runs the arrival block through the named pointer cell: it stores
 *           the freshly measured distance plus 0x400 back into halfword 0x3E,
 *           clears word 0x14, seeds word 0x20 with the signed -0x40 and
 *           decrements work byte 2, the step counter, without a lower bound.
 *           The arrival block samples the cell twice, once for the argument pair
 *           of its own distance call and again after that call for the store
 *           base, and the second sample serves every store of the block; the
 *           byte 2 read is hoisted above the two non-overlapping word stores.
 *           One local assigned from the cell before that store statement holds
 *           the base in $s0 across the call instead, which measured 31 of 43
 *           instructions 8 bytes under the original, and the named-cell
 *           spelling measured 43 of 43 at the original 172 bytes.
 *           The 0x18-byte frame keeps $ra and $s0 across the first call, and
 *           $s0 holds the signed halfword the comparison still needs after it
 *           returns. The function takes no arguments, returns nothing and
 *           writes no state outside the work record.
 *
 *           No jal names this address. The payload word at 0x800C21B4 holds it
 *           as entry 12 of the code pointer run based at 0x800C2180, the run
 *           the payload's own textbin table starts, so the prologue and the
 *           return path place this boundary; the roles of a dispatched entry
 *           are structural and no caller prototype is owed.
 *
 *           Its first 20 bytes are instruction-for-instruction identical to the
 *           exact sibling addWorkWord20ToWord14AccumulatePreviousWord14Into
 *           Halfword3EThenSetGlobalWorkByte1To5ClearGlobalWorkByte2AndInvoke
 *           HelperWhenHalfword3EAboveDistancePlus1000BossBoss02816_800C1F2C,
 *           which carries the same integration prefix statement for statement
 *           and parts from this body at the comparison, the slack 0x1000 and a
 *           different guarded block, and the mirror-cased sibling in
 *           BOSS054.EMI#16 addWorkWord20ToWord14AccumulatePreviousWord14Into
 *           Halfword3EThenAdvanceWorkByte2WhileBelowDistancePlus180 carries the
 *           same reset-free guard with the opposite comparison sense; the two of
 *           them supplied the statement order, the access widths, the double
 *           read of word 0x14 and the named-cell reload of the guarded block,
 *           and this body writes its own slack 0x400, its own block stores and
 *           its own counter direction from its own bytes. A scan of the 398
 *           extracted payloads under out/binaries found this 0xAC-byte span at
 *           its own offset only, so no other payload carries this body and those
 *           siblings are shape evidence, not clone sources.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulatePreviousWord14IntoHalfword3EThenStoreZeroInWord14AndMinus40InWord20DecrementWorkByte2WhenHalfword3EAboveRemeasuredDistancePlus400BossBoss02816_800C1E80(
    void) {
  u8 *work;

  work = D_1F800044;
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  if (*(s16 *)(work + 0x3E) >
      func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38)) + 0x400) {
    *(u16 *)(D_1F800044 + 0x3E) =
        func_8015477C(*(s32 *)(D_1F800044 + 0x34), *(s32 *)(D_1F800044 + 0x38)) +
        0x400;
    *(u32 *)(D_1F800044 + 0x14) = 0;
    *(u32 *)(D_1F800044 + 0x20) = -0x40;
    D_1F800044[2]--;
  }
}
