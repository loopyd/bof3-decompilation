#include "bof3/bof3.h"

/* This overlay's trail rebuild at 0x801F2F28: it shifts the 0x20-record trail
 * buffer passed in a0, reprojects every record from the work object published at
 * the scratchpad pointer cell and fills each record's heading and size. */
void advanceTrailProjectPointsFillHeadingsAndAverageAnglesWorld02Area08813_801F2F28(
    u8 *arg0);

/* Size seed of the trail buffer, the halfword entry 0x1F of the size run the
 * 0x801F2F28 walk copies into every record: this span seeds it with 0x60 before
 * its rebuild loop and entry 3 at 0x801F2E04 shrinks it by 6 per frame. It sits
 * in the fixed main-RAM trail area opened at 0x800E4800, so it stays
 * address-anchored. */
extern u16 D_800E4B3E; /* @source 0x800E4B3E @kind data */

/* Work-record pointer cell of the shared scratchpad, this target's map row for
 * 0x1F800044. It stays unqualified: this body derives a pointer from the cell
 * for the cursor seed and re-reads the cell for each of its two tail stores, and
 * the compiler must re-materialise the cell address for each access. */
extern u8 *scratchpadWorkRecord; /* @source 0x1F800044 @kind unknown */

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay's work-byte-1 handler table: the dispatcher
 * at 0x801F2C04 scales the work record's byte 0x01 by four and calls that entry
 * of the 0x801F58DC table, whose four words are this address, 0x801F2CE4,
 * 0x801F2D80 and 0x801F2E04. It first publishes the trail cursor: it takes the
 * work record through the scratchpad pointer cell 0x1F800044 and stores
 * 0x10000, 0x250000 and 0x3800000 into that record's words at +0x34, +0x38 and
 * +0x3C, the three cursor words the overlay walk at 0x801F2F28 copies into
 * record 0. It then seeds the trail buffer size halfword at the fixed main-RAM
 * address 0x800E4B3E with 0x60, the size the same walk copies into every record,
 * and rebuilds the whole trail buffer at the fixed main-RAM address 0x800E4800 -
 * the buffer every in-image call site of the walk passes - once per record: the
 * counter is a byte, incremented, masked to 0xFF and compared unsigned against
 * 0x20, so all 0x20 records take the seeded cursor before this entry returns. It
 * then reloads the work record through the cell, writes zero to that record's
 * byte 0x09 so the countdown restarts at full, and advances byte 0x01 by one
 * after reloading the cell a third time, so the next frame dispatches entry 1 at
 * 0x801F2CE4. It writes no 0x10 rearm: the reset leaves byte 0x09 at zero.
 * Takes no arguments and returns nothing; its 0x18-byte frame holds $s0 at 0x10
 * and $ra at 0x14, and $s0 carries the loop counter.
 *
 * Name evidence: no jal in the payload encodes this address - the only word
 * naming it is the table word at 0x801F58DC - so it is a dispatch entry, not a
 * called helper, and its index 0 is the work-byte-1 value that runs it. The one
 * callee supplies the rebuild verb, and this span's own stores supply the seed
 * facts: the three cursor words and the 0x60 size seed, both consumed by the
 * 0x801F2F28 walk. No byte-identical body exists anywhere in the corpus: a
 * search of all 398 payloads under out/binaries for these 0x9C bytes found only
 * this span at its own offset 0x48, and no payload carries the cursor triple, so
 * the clone lever is ineligible and this body is written from these bytes.
 *
 * Shape evidence: the two exact entries of the same table, 0x801F2D80 and
 * 0x801F2E04, supplied the declaration style - the local copy of the walk
 * prototype, the unqualified scratchpad cell extern, the tagged main-RAM
 * halfword extern and the PSX_PTR call argument. This body's own literals, its
 * word stores at the cursor offsets, the 0x60 seed, the loop bound and its two
 * tail stores come from the original bytes. The loop is written as a `for` over
 * a `u8` counter, the form whose tail these bytes show: the increment, then the
 * 0xFF mask and unsigned compare against the bound.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedTrailCursorAndSizeThenRebuildAllTrailRecordsResetCountdownAndAdvanceWorkByte1World02Area08813_801F2C48(
    void) {
  u8 *work;
  u8 i;

  work = scratchpadWorkRecord;
  *(u32 *)(work + 0x34) = 0x10000;
  *(u32 *)(work + 0x38) = 0x250000;
  *(u32 *)(work + 0x3C) = 0x3800000;
  D_800E4B3E = 0x60;
  for (i = 0; i < 0x20; i++) {
    advanceTrailProjectPointsFillHeadingsAndAverageAnglesWorld02Area08813_801F2F28(
        PSX_PTR(u8, 0x800E4800u));
  }
  scratchpadWorkRecord[9] = 0;
  scratchpadWorkRecord[1]++;
}
