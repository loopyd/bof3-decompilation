#include "bof3/bof3.h"
#include <rand.h>

/* Scratch work object published through the pointer cell at 0x1F800044. The
 * original materialises the cell as `lui 0x1F80` plus `lw 0x44(reg)` once,
 * before the two rand() calls, and keeps that pointer in $s3 for both
 * coordinate halves, so the named cell symbol is the binding. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared angular helpers: func_801783C8 and func_801782FC return the
 * 12-bit-scaled sine and cosine of a column angle handed in as $a0. */
s32 func_801783C8(s32 arg0);
s32 func_801782FC(s32 arg0);

/* @source 0x801F3D54
 * @behavior Seeds one 0xE-byte work record - the record kind the overlay's own
 *           ticker func_801F39D8 walks in the table at 0x800E4800 and hands to
 *           drawClampedWorkRecordVerticalBarWorld01Area06714_801F38A0 as a flat
 *           vertical bar. It draws a random column angle from `rand() & 0xFC0`
 *           and a random reach from `rand() & 0x38`, then stores, all as
 *           halfwords: the record's +0x0 x takes the scratch work object's
 *           halfword +0xC plus (cosine(angle) * reach) >> 12; its +0x2 y takes
 *           the work halfword +0xE plus (sine(angle) * reach) >> 13 and that
 *           same value is copied to the +0xA clamp; +0x4 half-height, +0x6
 *           velocity and +0x8 minimum clamp take zero; and +0xC takes 0x20, the
 *           countdown the ticker decrements before re-arming through this
 *           helper. The scratch work object is the one published at
 *           0x1F800044, read once into $s3 before the rand() calls. Caller
 *           evidence: the overlay's phase handler func_801F2F5C calls it at
 *           0x801F2FEC only with a record returned by
 *           findFreeWorkRecordByCountdownAt800E4800World01Area06714_801F3A9C,
 *           and the ticker calls it at 0x801F3A60 on the table entry whose +0xC
 *           countdown just reached zero (that ticker decrements +0xC by one,
 *           steps +0x2 down by +0x6, grows +0x4 and adds `countdown & 1` to
 *           +0x6 per frame), so the helper fills a claimed but uninitialised
 *           record. The 0xE-byte stride, the +0xC free marker and the 0x80-entry
 *           table come from the exact siblings clearWorkRecordCountdownsAt800E4800-
 *           World01Area06714_801F39AC and
 *           findFreeWorkRecordByCountdownAt800E4800World01Area06714_801F3A9C;
 *           the +0x0/+0x2/+0x4/+0x8/+0xA bar geometry comes from the sibling
 *           drawClampedWorkRecordVerticalBarWorld01Area06714_801F38A0. This
 *           helper takes the record in $a0, returns nothing and reads no other
 *           state; the 0x28-byte frame saves $ra and $s0-$s3 for the angle, the
 *           reach, the record pointer and the work pointer.
 * @status partial
 * @match 70.45
 * @residual 31/44 instructions, 176 -> 176 bytes. Every instruction is
 *           byte-identical except that this build puts the record pointer in
 *           $s1 and the reach in $s2, where the original puts the record in
 *           $s2 and the reach in $s1; the two registers swap in the prologue
 *           saves, the incoming-argument copy, both `mult`s, the seven `sh`
 *           stores and the epilogue restores, which is exactly the 13
 *           instructions the diff reports. Measured clean-C shapes that keep
 *           the payload's instruction order: the reach-first `mult` with a
 *           `s32` y copy for +0x2/+0xA reproduces the whole body including the
 *           delayed `li 0x20`; `u16` y, an extra `s32` x local, declaration
 *           permutations, `void*`/`u16*` record and the store orders
 *           8/4/6/2/A/C, 2/A/8/4/6/C, 8/4/6/C/2/A and 4/6/8/2/A/C all reproduce
 *           the same swap. `s32` angle drops the required second `andi 0xFFFF`
 *           (26/44, 172 bytes), so the u16 angle is proven. Compiler source
 *           (gcc 2.7.2 local-alloc.c) assigns callee-saved registers in qty
 *           priority order `floor_log2(n_refs) * n_refs * size / (death -
 *           birth)`, lowest free register first; every store references the
 *           record pointer, so its reference count and both live ranges are
 *           fixed by the payload, and no evidenced source shape moves the
 *           ordering. The register choice therefore needs the permuter rung,
 *           which requires parent authorization.
 */
void seedRandomWorkRecordPositionAroundWorkObjectAndArmCountdownWorld01Area06714_801F3D54(
    u8* record) {
  u8* work;
  u16 angle;
  s32 y;
  s32 reach;

  work = D_1F800044;
  angle = rand() & 0xFC0;
  reach = rand() & 0x38;
  FIELD_REF(u16, record, 0x0u) =
      FIELD_REF(u16, work, 0xCu) + ((reach * func_801783C8(angle)) >> 12);
  y = FIELD_REF(u16, work, 0xEu) + ((reach * func_801782FC(angle)) >> 13);
  FIELD_REF(u16, record, 0x8u) = 0;
  FIELD_REF(u16, record, 0x4u) = 0;
  FIELD_REF(u16, record, 0x6u) = 0;
  FIELD_REF(u16, record, 0x2u) = y;
  FIELD_REF(u16, record, 0xAu) = y;
  FIELD_REF(u16, record, 0xCu) = 0x20;
}
