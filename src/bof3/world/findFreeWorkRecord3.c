#include "bof3/bof3.h"

/* @source 0x801F2D7C
 * @behavior Free-record scan of the shared work table at 0x800E4800: it walks
 *           the lead byte of each of the 0x100 records of 0x18-byte stride and
 *           returns the address of the first record whose lead byte reads zero,
 *           or zero when all 0x100 records are claimed. It reads only each
 *           record's lead byte, writes nothing, calls nothing and keeps no
 *           register, so its 0x3C bytes carry no prologue and no frame. The
 *           bound test is the loop's back edge: the unsigned 16-bit counter is
 *           read back masked to 0xFFFF and compared below 0x100, the record
 *           pointer advance of 0x18 is scheduled into that back edge's delay
 *           slot, the exhausted path materialises zero in $v0, and a free
 *           record leaves the loop through the lead-byte branch with its copy in
 *           that branch's delay slot. The lead byte is the claim flag: this
 *           target's single in-image caller,
 *           spawnPairsActiveCountdowns,
 *           reaches it twice per active seed through the single jal at
 *           0x801F3108 inside a two-iteration loop, skips the record when the
 *           result is zero and otherwise stores 1 into that same lead byte in
 *           the call's delay slot before filling offsets 0x2, 0x3, 0x4, 0x8,
 *           0xC and 0x14.
 *
 * Name evidence: the role, the 0x800E4800 table and the lead-byte predicate are
 *           those of the exact siblings findFreeWorkRecordAt800E4800World01Area04913_801178C4,
 *           findFreeWorkRecordAt800E4800World03Area12513_801F3348 and
 *           findFreeWorkRecordAt800E4800World03Area13413_801F4938, whose 0x3C-byte
 *           bodies differ from this one in the mask immediate, the bound
 *           immediate and the stride immediate alone. The table extent and the
 *           0x18 stride are this target's own: the adjacent exact
 *           stateTableLeadBytes clears the
 *           three lead bytes of the same 0x100 entries of 0x18-byte stride. The
 *           0x3C bytes of this span equal the span at 0x801D171C of the payload
 *           emi/scenario/sce15ef3/00 (payload cmp of the two 60-byte spans, both
 *           hashing to 965d8a66ff2a453e1ce02ed318ee4956969be8e86a367c76600ee93ddd6bf7ee),
 *           whose own row there was still an asm boundary when this was lifted,
 *           so that twin supplied statement order and not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8* findFreeWorkRecord(void) {
  u8* record;
  u16 count;

  record = PSX_PTR(u8, 0x800E4800u);

  for (count = 0u; count < 0x100u; count++, record += 0x18u) {
    if (*record == 0) {
      return record;
    }
  }

  return 0;
}
