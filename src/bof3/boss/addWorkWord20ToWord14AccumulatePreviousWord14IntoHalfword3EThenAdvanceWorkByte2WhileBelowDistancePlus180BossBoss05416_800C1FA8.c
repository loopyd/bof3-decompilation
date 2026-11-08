#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, bound by its own D_ name and not by
 * SPAD_PTR_SLOT: the original materializes the base for every access
 * (lui $v0,0x1F80 / lw $v0,68($v0), and again through $v1 before the byte
 * store), while the macro's absolute-constant lvalue makes the compiler hold
 * 0x1F800044 in a callee-saved register and load through offset zero. The exact
 * bmagic sibling addWorkWord20ToWord14AccumulateWord14IntoHalfword3E...
 * _801EFC54 uses this same named-cell spelling for the same reload pattern. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared main-program helper the whole boss archive family calls: it returns
 * the distance between the fixed-point position pair it takes as (x, z), and
 * exact siblings declare it `s16` where they store or shift its result. This
 * target's only use compares the whole returned word against a signed halfword
 * without a sign-extension pair, so the return type is the full `s32` here and
 * the width belongs to this use site. This payload's regenerated
 * undefined_funcs list still carries the func_8015477C row, so the callee needs
 * no target map row. */
extern s32 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x800C1FA8
 * @behavior Per-frame field integrator of this payload's work state machine: it
 *           loads the work record pointer from the scratchpad pointer cell
 *           0x1F800044 once and adds its unsigned halfword 0x3E by its word
 *           0x14 as it stood before this update, then advances that word 0x14
 *           by word 0x20. It re-reads halfword 0x3E as a signed value, takes
 *           the func_8015477C distance of the record's position word pair at
 *           0x34 and 0x38, and while the accumulated halfword is below that
 *           distance plus 0x180 it re-reads the pointer cell and raises work
 *           byte 2, the handler index, with a byte-width read-modify-write. The
 *           word 0x14 read for the halfword update is the one the original
 *           loads before itself stores word 0x14, which is why the body reads
 *           word 0x14 twice; the 0x18-byte frame keeps $ra and $s0 across the
 *           one call, and the function takes no arguments and returns nothing.
 *           No jal names 0x800C1FA8: its address is the word at 0x800C1830,
 *           entry 11 of the overlay header pointer run 0x800C1804 through
 *           0x800C1834, so the main program calls it directly and the prologue
 *           and return path place the boundary. Its first 72 bytes are
 *           instruction-for-instruction identical to the boss028 payload's span
 *           func_800C1F2C, which shares the whole integration prefix and parts
 *           from this body at that span's 0x800C1F78 with the opposite
 *           comparison sense, the immediate 0x1000 and a different guarded
 *           block, so that span supplies shape and not bytes; the halfword 0x3E
 *           accumulation form itself is the one the exact bmagic sibling
 *           addWorkWord20ToWord14AccumulateWord14IntoHalfword3E..._801EFC54
 *           uses, read here on the pre-update value because the order of the
 *           two stores puts it there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulatePreviousWord14IntoHalfword3EThenAdvanceWorkByte2WhileBelowDistancePlus180BossBoss05416_800C1FA8(
    void) {
  u8 *work;
  u8 *state;

  work = D_1F800044;
  *(u16 *)(work + 0x3E) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  if (*(s16 *)(work + 0x3E) <
      func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38)) + 0x180) {
    state = D_1F800044;
    state[2]++;
  }
}
