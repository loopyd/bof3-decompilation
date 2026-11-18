#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, bound by its own D_ name and not by
 * SPAD_PTR_SLOT: the original materialises the base once (lui $v0,0x1F80 /
 * lw $v0,68($v0)) ahead of its own frame setup and reaches every record access
 * through that one register, while the macro's absolute-constant lvalue makes
 * the compiler hold 0x1F800044 in a callee-saved register and load through
 * offset zero. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay's image;
 * this target's own generated undefined_syms_auto.txt already binds the
 * address, and every access here is byte-addressed, so the declaration keeps
 * the byte view only. The roles of its bytes +1 and +2 are not established, so
 * they stay offset-anchored. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-program helper the whole boss archive family calls: it returns
 * the distance between the fixed-point position pair it takes as (x, z). This
 * use adds the slack 0x1000 to the returned word and compares it against a
 * signed halfword without a sign-extension pair, so the return type is the full
 * `s32` here and the width belongs to this use site. This target's generated
 * undefined_funcs_auto.txt binds the address, so it needs no map row. */
extern s32 func_8015477C(s32 arg0, s32 arg1);

/* Shared main-executable helper of this overlay's step, called without
 * arguments. Its role is not established, so it keeps its address-anchored
 * spelling; the target map already carries its row, backed by this declared
 * prototype. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* @source 0x800C1F2C
 * @behavior Per-frame field integrator of this payload's boss step state: it
 *           loads the work-record pointer from the scratchpad pointer cell
 *           0x1F800044 once, ahead of its own frame setup, and performs two
 *           read-modify-writes through that single loaded pointer. It first
 *           adds the record's unsigned halfword 0x3E by its word 0x14 as that
 *           word stood before this update, then advances word 0x14 by word
 *           0x20; the word 0x14 the halfword update consumes is the original's
 *           second load of that address rather than a copy of the value the
 *           update stores, because the halfword statement precedes the update
 *           in the body and its own store may alias the word. It re-reads
 *           halfword 0x3E as a signed value, takes the func_8015477C distance
 *           of the record's position word pair at 0x34 and 0x38, and when that
 *           distance plus 0x1000 is still below the accumulated halfword, it
 *           stores 5 into byte +1 of the shared battle work record at
 *           0x801EB4E0, clears that record's byte +2 and calls the shared
 *           helper 0x801E5988, the clear filling the call's delay slot. Each of
 *           those two record accesses re-reads the global pointer cell, because
 *           the byte store between them may alias it. The 0x18-byte frame keeps
 *           $ra and $s0 across the one call, and $s0 holds the signed halfword
 *           the comparison still needs after it returns. The function takes no
 *           arguments, returns nothing and writes no other state.
 *
 *           No jal names this address. The payload word at 0x800C21B8 holds it
 *           twice over: entry 8 of the handler run based at 0x800C2198 that
 *           dispatchIndexedHandlerBossBoss02816_800C1AF0 indexes, and slot 3 of
 *           the seven-slot run based at 0x800C21AC that
 *           dispatchWorkByte2Table21acThenInvokeSharedServiceWhenWorkByte0Bit0Set
 *           BossBoss02816_800C1C30 indexes, so the prologue and the return path
 *           place this boundary.
 *
 *           Its first 0x48 bytes are instruction-for-instruction identical to
 *           the exact sibling addWorkWord20ToWord14AccumulatePreviousWord14Into
 *           Halfword3EThenAdvanceWorkByte2WhileBelowDistancePlus180
 *           BossBoss05416_800C1FA8, which carries the same integration prefix
 *           and takes the opposite comparison sense, the slack 0x180 and a work
 *           byte 2 advance on its guard; that sibling supplied the statement
 *           order, the access widths and the double read of word 0x14, and this
 *           body writes its own slack, its own guard polarity and its own
 *           guarded block from these bytes. A scan of the 398 extracted payloads
 *           under out/binaries matched this span only at its own offset, both
 *           with the two jal immediates masked and at opcode/register
 *           granularity with every immediate dropped, so no other payload
 *           carries this body and the sibling is shape evidence, not a clone
 *           source.
 *
 *           The comparison's operand order is load-bearing. With the call
 *           first (`func_8015477C(...) + 0x1000 < *(s16 *)(work + 0x3E)`) the
 *           build measured 18 of 35 instructions at four bytes under: the
 *           signed halfword load sank below the call, so the record pointer
 *           outlived it and took a callee-saved register, and the word 0x14
 *           store fell into the call's delay slot. Writing the halfword on the
 *           left, as the original's own `lh` ahead of the `jal` shows, measured
 *           35 of 35 at the payload's 140 bytes. A probe batch of four
 *           statement shapes measured 35/35 both for this spelling and for the
 *           same body with the signed halfword held in an explicit `s16`
 *           local, so the extra local is not the lever and this body keeps the
 *           direct read it uses; a word-0x14-first order with a `u32` old-value
 *           local measured 20 of 35 and naming the two record-pointer reads in
 *           a local measured 28 of 35.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulatePreviousWord14IntoHalfword3EThenSetGlobalWorkByte1To5ClearGlobalWorkByte2AndInvokeHelperWhenHalfword3EAboveDistancePlus1000BossBoss02816_800C1F2C(
    void) {
  u8 *work;

  work = D_1F800044;
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  if (*(s16 *)(work + 0x3E) >
      func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38)) + 0x1000) {
    D_801EB4E0[1] = 5;
    D_801EB4E0[2] = 0;
    func_801E5988();
  }
}
