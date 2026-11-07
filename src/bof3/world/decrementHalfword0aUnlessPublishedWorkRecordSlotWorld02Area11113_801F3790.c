#include "bof3/bof3.h"

/* 152-byte (0x98) work-record cell of the shared table at 0x80147A58. Only
 * the stride is proven here: the original scales the distance between the
 * published record and that table by exactly 0x98, which is the same stride
 * the resident round-robin code uses for these cells. */
typedef struct World02Area11113WorkRecord {
  u8 bytes[0x98];
} World02Area11113WorkRecord;

/* Pointer cell of the published 152-byte work record. */
extern World02Area11113WorkRecord* D_80146884; /* @source 0x80146884 @kind unknown */

/* Base of the shared 0x98-byte work-record table. */
extern World02Area11113WorkRecord D_80147A58[]; /* @source 0x80147A58 @kind bss */

/* Shared state byte of this overlay's step family. The preceding handler in
 * the same payload (0x801F3768) seeds it with `0x10 << work[0x18]`, and this
 * body compares it against the same 0x10-based code for the published record's
 * slot, which is why the byte stays address-anchored. */
extern u8 D_80146867; /* @source 0x80146867 @kind unknown */

/* Pointer cell of the shared record whose halfword at 0x0A this step consumes;
 * the family's other steps take two units off the same cell. */
extern u16* D_80148210; /* @source 0x80148210 @kind unknown */

/* @source 0x801F3790
 * @behavior Area step that returns without touching the shared record while
 *           the resident state byte at 0x80146867 already reads as the slot
 *           code of the 152-byte work record published through the pointer
 *           cell 0x80146884. That code is 0x10 shifted left by the record's
 *           index in the shared table at 0x80147A58 (the pointer difference
 *           against that table, 0x98-byte stride). When the state byte
 *           differs, the step consumes two units of the halfword at 0x0A of
 *           the record published through the global pointer cell 0x80148210.
 *           Takes no arguments, calls nothing and returns nothing.
 *
 * Evidence: the payload span 0x801F3790-0x801F3807 is one complete function
 * with no prologue, a single body and the one epilogue `jr $ra; nop`, so it is
 * a leaf, not a fragment. The original loads the state byte with `lbu`, forms
 * the record distance with a bare `subu` against the address of 0x80147A58
 * (`lui %hi`/`addiu %lo`), divides it by 0x98 through the compiler's
 * pointer-difference scale (the exact-division shift-add chain for the
 * modular inverse of 19 followed by a three-bit shift), then builds
 * `0x10 << index` with `addiu $v0,$zero,0x10` plus `sllv` and compares it
 * against the state byte with `beq`; only the fall-through path loads the
 * pointer cell 0x80148210, takes `lhu 0xA` and stores it back after
 * `addiu -2`. No in-payload word holds 0x801F3790, so the body is reached
 * from outside this overlay and has no local caller.
 *
 * The scale uses unsigned multiplication by 0x286BCA1B, the inverse of 19
 * modulo 2^32, followed by a logical shift by three. For table records this
 * gives the 0x98-byte slot index while preserving the original unsigned
 * shift, rather than the signed shift of a C record-pointer difference.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementHalfword0aUnlessPublishedWorkRecordSlotWorld02Area11113_801F3790(
    void) {
  u32 slot;

  slot = (((u32)((u8*)D_80146884 - (u8*)D_80147A58)) * 0x286BCA1Bu) >> 3;
  if (D_80146867 != (0x10 << slot)) {
    D_80148210[5] -= 2;
  }
}
