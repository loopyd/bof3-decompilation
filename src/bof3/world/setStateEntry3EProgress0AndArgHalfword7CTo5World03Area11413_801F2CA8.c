#include "bof3/bof3.h"

/* @source 0x801448EB @kind unknown */
extern u8 D_801448EB;
/* @source 0x801448EC @kind unknown */
extern u8 D_801448EC;

void func_8015C088(void);

/* @source 0x801F2CA8
 * @behavior Runs the shared front-end startup helper at 0x8015C088, writes 0x3E
 *           to the state-entry byte 0x801448EB and zero to the progress byte
 *           0x801448EC, stores 5 into the halfword at 0x7C of the record passed
 *           in $a0 and returns 0. The 0x18-byte frame keeps $ra and $s0 across
 *           the call, the argument copy into $s0 fills the jal delay slot, each
 *           byte store materialises the 0x8014 page in $at on its own, and the
 *           epilogue restores $ra and $s0 before `jr $ra` leaves a nop in the
 *           delay slot. No jal and no payload word addresses 0x801F2CA8: the
 *           overlay's leading tabledata run at file offsets 4..0x64 holds only
 *           the distinct words 0x801F2D24, 0x801F2D3C, 0x801F2D58,
 *           0x801F2D94, 0x801F2DD8, 0x801F2E38, 0x801F2E88, 0x801F2E90,
 *           0x801F2ED4 and 0x801F2F50, so no word names this span, and the
 *           overlay loader or a script blob reaches this handler from outside
 *           the payload, so its single argument role stays structural.
 *
 * Name evidence: 0x801448EB is the shared state-entry byte and 0x801448EC its
 * progress byte, the pair the exact World03 area143, World04 area167/168/171
 * and world initialisers write through func_8015C088; the in-target reviewed
 * candidate initArea11413State@0x801F2C68 writes the same two bytes from the
 * same helper. The halfword at +0x7C is the record field the in-target
 * func_801F2CF0 also seeds (with 9 at 0x801F2E34, through the pointer it loads
 * from D_80148208), so the store is named by its anchored offset because no
 * lifted source gives that field a semantic role.
 *
 * Shape and lever ledger: the body was seeded directly from the original bytes
 * (prologue, ABI, literals 0x3E and 5, halfword store and the explicit zero
 * return all read from the payload). Declined levers: a scan of every extracted
 * payload under out/binaries for these 72 bytes returned only this span's own
 * offset, so no byte-identical twin exists and clone is ineligible; the span
 * opens with its own `addiu $sp,-0x18` and closes with its own `jr $ra`, so no
 * fragment boundary is carved; the range is code that no table word or
 * consumer indexes, so it is not data. Probes measured through the sweeper with
 * the owning profile fixed: parameter forms `u8 *`, `void *` cast and `s32`;
 * store orders EB/EC/field, EC/EB/field and field first; a reused narrow `u8`
 * or `s32` value local across both literals; a `u16 *` field local; unsigned
 * literals with a cast return; `volatile` externs and a `volatile` field store;
 * explicit return locals assigned before the call, after it, after the first
 * store and last, including a two-definition return local; and return types
 * s8/u8/s16/u16/u32/s32/void *. All measured 13 of 18 instructions at 72 bytes
 * with the same first difference at +0x14, except the `void` form at 68 bytes
 * and the late re-stored return local at 80 bytes, which regressed and are not
 * retained.
 * @status partial
 * @match 72.22
 * @residual The original keeps both literals in $v1 (li v1,0x3E, li v1,5) and
 *           materialises the constant-zero return in $v0 immediately after the
 *           jal, while every measured clean-C shape allocates the literals to
 *           $v0 and emits the zero return last; the two facts are coupled, the
 *           size, frame and store order already agree, and this is the same
 *           constant-scratch class the reviewed in-target candidate
 *           initArea11413State@0x801F2C68 retains at 68.75 percent.
 */
s32 setStateEntry3EProgress0AndArgHalfword7CTo5World03Area11413_801F2CA8(u8 *arg0) {
  func_8015C088();
  D_801448EB = 0x3E;
  D_801448EC = 0;
  *(u16 *)(arg0 + 0x7C) = 5;
  return 0;
}
