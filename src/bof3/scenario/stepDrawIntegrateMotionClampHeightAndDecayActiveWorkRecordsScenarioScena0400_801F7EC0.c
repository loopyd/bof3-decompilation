#include "bof3/bof3.h"

void func_801F7CF0(u8* record);

/* @source 0x801AFE18 @kind unknown
 * Shared projection setup: it consumes the caller's 0x20-byte scratch buffer
 * and publishes the projection the per-record sprite emitter reads.
 */
extern void func_801AFE18(void* arg0);

/* @source 0x8015477C @kind unknown
 * Shared fixed-point distance helper over the word pair passed as (x, z);
 * callers publish its result scaled by 0x10000 and it answers a halfword.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x80178818 @kind unknown
 * Shared in-place direction helper: it takes one three-word direction triple
 * twice and rewrites it where it stands.
 */
extern void func_80178818(void* arg0, void* arg1);

/* @source 0x801F7EC0
 * @behavior Per-frame pass over the 0x80-entry, 0x28-byte-stride work table at
 *           0x800E4800, which the four seeding handlers of this overlay call
 *           after they claim records. It fills the 0x20-byte projection
 *           scratch at $sp+0x10 through the shared projection setup at
 *           0x801AFE18 first, then walks the table with an unsigned byte
 *           counter and, for every entry whose claim byte at offset 0x25 reads
 *           non-zero, emits that entry's sprite through the in-payload emitter
 *           at 0x801F7CF0 with the entry base in $a0, integrates the position
 *           words at +0x00, +0x04 and +0x08 by the velocity words at +0x10,
 *           +0x14 and +0x18, and asks the shared distance helper 0x8015477C for
 *           the planar distance of the new +0x00/+0x04 pair, sign-extended
 *           from its halfword answer.
 *           When the +0x08 word reads below that distance it is raised to the
 *           distance and the +0x18 word is cleared, then the direction triple
 *           at +0x10 is rewritten in place by the shared direction helper
 *           0x80178818 with the cleared +0x18 word filling that call's delay
 *           slot. The decay follows on both arms: the life byte at +0x24 drops
 *           by one, the word at +0x20 gains four and the claim byte at +0x25
 *           drops by one, so an entry releases itself after as many passes as
 *           the countdown its seeder wrote.
 *           It takes no arguments, consumes no result, and its 0x40-byte frame
 *           holds the 0x20-byte projection scratch above the saved $ra and
 *           $s0-$s2.
 *
 * Matching note: the record is reached through one base pointer; holding the
 *           claim byte in its own pointer instead measured 48-51/72 at 288
 *           bytes, and the two-pointer seeds also differ at the first
 *           instruction. Ordering the clamp site as raise, clear, call measured
 *           61/67 with first=+0x00b8, while raising and clearing before the
 *           call left the same body at 61/67 with the cleared word in the call
 *           delay slot. The decay block is what placed the last six
 *           instructions: as three compound assignments it measured 61/67 at
 *           first=+0x00b8, and carrying the three results in locals and storing
 *           them in the same order measured 67/67 at the unchanged 268 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepDrawIntegrateMotionClampHeightAndDecayActiveWorkRecordsScenarioScena0400_801F7EC0(
    void) {
  u8 scratch[0x20];
  u8* base;
  u8 index;
  u8 life;
  u8 claim;
  s32 age;
  s32 dist;

  func_801AFE18(scratch);
  base = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  do {
    if (base[0x25] != 0) {
      func_801F7CF0(base);
      *(s32*)(base + 0x00) += *(s32*)(base + 0x10);
      *(s32*)(base + 0x04) += *(s32*)(base + 0x14);
      *(s32*)(base + 0x08) += *(s32*)(base + 0x18);
      dist = func_8015477C(*(s32*)(base + 0x00), *(s32*)(base + 0x04));
      if (*(s32*)(base + 0x08) < dist) {
        *(s32*)(base + 0x08) = dist;
        *(s32*)(base + 0x18) = 0;
        func_80178818(base + 0x10, base + 0x10);
      }
      life = base[0x24] - 1;
      age = *(s32*)(base + 0x20) + 4;
      claim = base[0x25] - 1;
      base[0x24] = life;
      *(s32*)(base + 0x20) = age;
      base[0x25] = claim;
    }
    base += 0x28;
    index += 1u;
  } while (index < 0x80u);
}
