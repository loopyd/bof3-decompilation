#include "bof3/bof3.h"

extern u8 D_801454F2;

/* The local drawer narrows both arguments with `sll`/`sra 16` in its own
 * prologue and neither call site consumes a result. */
void func_801F4664(s16 arg0, s16 arg1);

/* @source 0x801F411C
 * @behavior When the shared byte at 0x801454F2 is 2, writes 3 to byte 0x02 of
 *           the work record published at the scratchpad pointer slot 0x44.
 *           Then reads that cell again and calls the local 0x801F4664 drawer
 *           with 0x10 and the signed halfword at record offset 0x2E. The
 *           pointer cell is read once per group, so the arm store and the
 *           drawer argument each carry their own cell load. It is the
 *           step-forward handler of the byte-2 series: 0x801F40C0 advances
 *           work halfword 0x2E by 0x10 and calls this function, and the
 *           0x801F416C retreat handler shares this function's drawer tail.
 *           The 0x50 bytes are byte-identical to the exact
 *           setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld03Area12114_801F35D0
 *           in emi/world03/area121/14 apart from the two low bytes of the
 *           internal `jal` word that encode that overlay's own drawer address,
 *           so this body follows the reviewed sibling's statement order with
 *           this target's drawer. It takes no arguments, uses no result, saves
 *           only the return address and touches no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkState2To3When454f2Is2ThenDrawHalfword2eWorld02Area08813_801F411C(
    void) {
  u8* record;

  if (D_801454F2 == 2) {
    SPAD_PTR_SLOT(u8, 0x44)[2] = 3;
  }
  record = SPAD_PTR_SLOT(u8, 0x44);
  func_801F4664(0x10, FIELD_REF(s16, record, 0x2E));
}
