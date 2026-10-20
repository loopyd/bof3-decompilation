#include "bof3/world/area01613_internal.h"

/* @source 0x801F30EC
 * @behavior refreshes the scratch byte at offset `0x07` from offset `0x0B`, then
 * when that byte is `1` searches the local seed record table at `0x801F4D7C`
 * for the record whose mask matches the shared seed word at `0x80143F10`, seeds
 * offsets `0x18`/`0x40`/`0x44`/`0x48`/`0x09` and calls the shared
 * `0x8014D6B8` helper with the record byte at offset `2`; scratch bytes `2`, `3`
 * and `4` seed the same offsets and call that helper with `3`, `0` and `1`, and
 * any other byte returns without a call; every calling path sets the mode byte
 * at offset `0x01` to `2`.
 * @status partial
 * @match 58.97
 * @residual first diff +0x0034: the 0x801F4D7C record load and the arm-1 store block split, and arm-1's 0x40/0x44/0x48/0x09 suffix is cross-jumped into the shared tail; the search loop also lacks the original's `move v1,a1` index copy. Rung 4 is measured negative: `lift flag-search` over 52 catalogue profiles returns no exact match and keeps the -O2/-G0 baseline (58.97) above the best alternative (-O1/-G4 at 60.00 with other hunks broken). Four further live variants that reorder the arm-1 stores or force scratch reloads in that arm stay at 46/78 and only shift the emitted size (268 -> 276/284 bytes against 312 original), so the miss is the original's block placement - its found block sits between the case-3 and case-4 tests - plus the missing `move v1,a1` loop-index copy. The permuter rung is opt-in and was not authorised for this selector.
 * Volatile-pointee qualification is measured negative in this repair round: the
 * reviewed full shape (scratch/reload/mode volatile) gives 41/78 at 288 bytes,
 * reload+mode 41/78, and mode alone 43/78 at 284 bytes, because the regained
 * `0x0B` reload displaces the cell reload ahead of it into `$a0` and moves the
 * first difference earlier to `+0x0020`, so all three shapes were reverted.
 * The reviewed store-block-merge-break lever is measured negative in both shapes it
 * has: routing the else-arm `0x40`/`0x44`/`0x48` stores through the non-volatile
 * `mode` view gives 36/78 at 284 bytes with the first difference at `+0x0000`, and
 * keeping `target = D_1F800044` live while only `0x44`/`0x48` go through `mode` gives
 * 37/78 at 292 bytes with the same `+0x0000`; both disturb the entry schedule that
 * this candidate matches byte for byte and were reverted, so the cross-jump out of
 * the arm-1 store block is still open for an allocator-level change.
 * The reviewed cell-volatility-view-split lever (repair 4) is measured negative too:
 * reading the else-arm state byte through the volatile pointer-object cell and
 * storing through the non-volatile `mode` view gives 40/78 at 304 bytes with the
 * first difference still at `+0x0034` and the entry block intact, so the regained
 * `0x0B` reload costs six matched instructions instead of restoring arm-1's stores.
 * The read axis and the store axis are both now measured, leaving the block
 * placement to an allocator-level change.
 * The reviewed scratch-cell-volatile-reload lever (repair 5) is measured negative
 * in three shapes: a live `cell` local so all seven acquisitions reload gives
 * 26/78 at 268 bytes with the first difference at `+0x0000` because the entry
 * cell load sinks below the frame setup and the frame grows to 32 bytes; the
 * same reloads written as `PSX_REF(World00Area016Scratch* volatile, ...)` with the
 * entry kept on this header binding give 24/78 at 280 bytes (first difference
 * `+0x0020`), and with the entry reloading too 24/78 at 280 bytes (`+0x0000`).
 * The reviewed loop-index-copy lever is measured neutral: copying `offset` into
 * `scan` at the loop top leaves 46/78 at 268 bytes and the same `+0x0034`, so the
 * copy is propagated away and the loop's `move $v1,$a1` is not reachable from
 * statement order. Every qualifier and statement-order spelling on this function
 * is now measured; the remaining hunks are the original's block placement and
 * its allocator choices.
 */
void func_801F30EC(void) {
  World00Area016Scratch* scratch;
  volatile World00Area016Scratch* target;
  World00Area016Scratch* reload;
  u32                     flag;
  u32                     state;
  u32                     value;

  scratch = D_1F800044;
  scratch->unk_04[3] = scratch->unk_0b;

  reload = D_1F800044;
  value = reload->unk_0b;
  if (value == 1) {
    u32 offset;
    u32 seed;

    seed = D_80143F10;
    offset = 0;
  search_seed:
    if (FIELD_REF(const u16, D_801F4D7C, offset) == seed) {
      goto seed_found;
    }
    offset += 4;
    goto search_seed;
  seed_found:
    target = D_1F800044;
    state = 2;
    target->unk_18 = seed;
    target->unk_40 = 0;
    target->unk_44 = 0x10000;
    target->unk_48 = state;
    D_1F800044->unk_04[5] = 8;
    flag = FIELD_REF(const u8, D_801F4D7C, offset + 2);
  } else {
    World00Area016Scratch* mode;
    u32                     state_byte;

    mode = D_1F800044;
    state_byte = mode->unk_0b;
    if (state_byte == 2) {
      flag = 3;
    } else if (state_byte == 3) {
      flag = 0;
    } else if (state_byte == 4) {
      flag = 1;
    } else {
      return;
    }

    target = D_1F800044;
    state = 2;
    target->unk_40 = 0;
    target->unk_44 = 0x10000;
    target->unk_48 = state;
    D_1F800044->unk_04[5] = 8;
  }

  func_8014D6B8(flag);
  D_1F800044->mode = state;
}
