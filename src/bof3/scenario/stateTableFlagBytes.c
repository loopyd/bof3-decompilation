#include "bof3/bof3.h"

/* @source 0x801F8358
 * @behavior Overlay table-clearing helper: it zeroes the lead byte of each of
 * the 0x8 entries of the 0x1C-byte-stride record table at 0x800E4800 (the
 * main-RAM work area this overlay shares with the world and scenario overlays),
 * then zeroes the two payload flag bytes at 0x801FDFC4 and 0x801FDFC8. The
 * clear is a bottom-tested stride walk: the unsigned byte counter is read back
 * masked to 0xFF and compared below 0x8, the 0x1C record-pointer advance is
 * scheduled into that back edge's delay slot, and the counter increment is the
 * last word of the loop body. The lead byte it clears is the claim flag the
 * same-table scan at 0x801F8394 tests, so the pair empties the record pool and
 * then drops this overlay's two flags. Both fixed addresses are payload- or
 * main-RAM-absolute and each store materialises its own base, so the two flag
 * stores keep separate constant loads. It reads no argument, returns nothing,
 * makes no call and keeps nothing live, so its 0x3C bytes carry no frame and the
 * $ra return is immediate. Its only in-payload references are two calls,
 * jal 0x801F8358 at 0x801F6E5C inside
 * stateTableIncrementCountdown and
 * jal 0x801F8358 at 0x801F7A6C inside
 * stateDistanceTransitionArm;
 * no image word holds its address.
 *
 * Representation notes. The loop was seeded from the same target's
 * stateWorkTable (base 0x800E515C, stride
 * 0x20, 0x40 entries), whose metadata records it exact, because this span's
 * loop is that body instruction for instruction with only those immediates
 * changed; the two trailing flag clears are this span's own bytes. The
 * claim-flag reading of the lead byte is anchored on the already-exact
 * sibling findFreeWorkRecordScenarioScena0700_801F8394, which walks the same
 * table with the same counter and pointer advance. The clone lever was
 * declined: no byte-identical twin of this span exists, because the two flag
 * stores name addresses inside this payload's own range.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTableFlagBytes(
    void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x1Cu;
    i += 1u;
  } while (i < 0x8u);

  *PSX_PTR(u8, 0x801FDFC4u) = 0;
  *PSX_PTR(u8, 0x801FDFC8u) = 0;
}
