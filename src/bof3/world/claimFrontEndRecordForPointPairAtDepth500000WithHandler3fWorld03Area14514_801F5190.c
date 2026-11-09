#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. The original re-materialises the row base for every
 * store and adds the scaled index to it, so each written field keeps its own
 * absolute symbol instead of one record view.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte +5, the front-end entry table handler selector this body sets to
 * 0x3F.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];
/* Row word +0x0C, the first coordinate of the leading point.
 * @source 0x80143FD4 @kind unknown
 */
extern u8 D_80143FD4[];
/* Row word +0x10, the second coordinate of the leading point.
 * @source 0x80143FD8 @kind unknown
 */
extern u8 D_80143FD8[];
/* Row word +0x14, the depth word of the leading point; this body writes the
 * fixed 0x500000.
 * @source 0x80143FDC @kind unknown
 */
extern u8 D_80143FDC[];
/* Row word +0x18, the first coordinate of the trailing point.
 * @source 0x80143FE0 @kind unknown
 */
extern u8 D_80143FE0[];
/* Row word +0x1C, the second coordinate of the trailing point.
 * @source 0x80143FE4 @kind unknown
 */
extern u8 D_80143FE4[];
/* Row word +0x20, the depth word of the trailing point; this body writes the
 * fixed 0x500000.
 * @source 0x80143FE8 @kind unknown
 */
extern u8 D_80143FE8[];

/* Scratchpad byte 0 holds the last claimed front-end record index: every
 * seeder of this family stores it there and later row addresses re-read it.
 * @source 0x1F800000 @kind unknown
 */
extern u8 D_1F800000;

/* Front-end record allocator of the shared main executable: it answers the
 * index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when every row is
 * occupied. */
u8 func_8019601C(void);

/* @source 0x801F5190
 * @behavior Claims the next free 0x74-byte front-end record row of the shared
 *           table at 0x80143FC8 through the allocator func_8019601C, publishes
 *           the returned index in scratchpad byte 0x1F800000 and, unless that
 *           index is 0xFF, seeds the claimed row: byte +0 takes the occupancy
 *           flag 1, byte +5 the handler selector 0x3F, the words at +0x0C and
 *           +0x10 the first 16.16 point it is given, +0x18 and +0x1C the
 *           second, and both depth words +0x14 and +0x20 the fixed 0x500000. It
 *           answers 1 when the table is full and 0 once the row is seeded. Its
 *           one in-image caller is the stage walker at 0x801F504C, which passes
 *           the shared work-record position and that position advanced by the
 *           stage step times the elapsed value, so the two points it queues span
 *           one stage segment at the fixed depth.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Name evidence: the allocator func_8019601C and the 0x74-byte row table at
 * 0x80143FC8 are the front-end record family that the exact lifts
 * claimFrontEndRecordHandler0BEveryEighthFrameWhileModeAtLeast7World00Area00113_801F2CD4
 * and claimFrontEndRecordHandler34FromPositionPairStoreStateDistanceAndClearRowByteBPlcharPlp46700_801D010C
 * already document, including row byte +5 as the handler selector and byte +0 as
 * the occupancy flag. Row words +0x0C/+0x10 and +0x18/+0x1C are two 16.16
 * coordinate pairs and +0x14/+0x20 their depth words, which this body only ever
 * writes as the fixed 0x500000; no consumer of the trailing pair exists in the
 * lifted corpus, so those fields stay described by offset and value.
 *
 * Shape notes from the original bytes: the claim block is the family's exact
 * form - `andi $v1,$v0,0xFF`, `addiu $v0,$zero,0xFF`, then the branch with the
 * compiler's 0x74 index scale `sll 3 / subu / sll 2 / addu / sll 2` started in
 * its delay slot. The first row store reuses the allocator result through the
 * index local, and every later row address re-reads the scratchpad byte
 * (`lbu $v1,0x0($a0)` with its load-delay nop), which is why the source keeps
 * them on `*scratch`. The two depth stores share one computed offset in $v1, so
 * the tail names the product once. Arm order is the one layout lever here: with
 * the seeding work and its `return 0` inside the guarded block and the `return
 * 1` after it, the original's `bne` to the work arm, `j` to the epilogue and
 * `li $v0,1` delay slot all reproduce; an early `return 1` before the work
 * relocated that arm out of line instead, at one extra instruction and four
 * bytes. The 0x28-byte frame keeps $s0-$s3 and $ra, one saved register per
 * argument, because all four are stored after the call.
 */
s32 claimFrontEndRecordForPointPairAtDepth500000WithHandler3fWorld03Area14514_801F5190(
    s32 x0, s32 y0, s32 x1, s32 y1) {
  u32 offset;
  u8* scratch;
  u8 index;

  index = func_8019601C();
  scratch = &D_1F800000;
  *scratch = index;

  if (index != 0xffu) {
    D_80143FC8[(u32)index * 0x74u] = 1u;
    D_80143FCD[(u32)*scratch * 0x74u] = 0x3fu;
    *(s32*)(D_80143FD4 + (u32)*scratch * 0x74u) = x0;
    *(s32*)(D_80143FD8 + (u32)*scratch * 0x74u) = y0;
    *(s32*)(D_80143FE0 + (u32)*scratch * 0x74u) = x1;
    *(s32*)(D_80143FE4 + (u32)*scratch * 0x74u) = y1;
    offset = (u32)*scratch * 0x74u;
    *(s32*)(D_80143FE8 + offset) = 0x500000;
    *(s32*)(D_80143FDC + offset) = 0x500000;
    return 0;
  }
  return 1;
}
