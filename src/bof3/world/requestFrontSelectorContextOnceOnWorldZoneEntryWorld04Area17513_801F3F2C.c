#include "bof3/bof3.h"

/* One-shot latch re-armed by every position outside the trigger rectangle and
 * consumed by the entry that fires. The area-module entries of the world04
 * overlays 182, 183 and 184 also clear this byte at start-up, so it belongs to
 * the overlay-wide request latch rather than to this body alone; no map names
 * it, so the spelling stays address-anchored. */
extern u8 D_801455C7; /* @source 0x801455C7 @kind data */

/* Shared front selector bundle filled elsewhere in the main executable: the
 * selector seed halfword and the two 16.16 context words the front-end request
 * below publishes. Read here exactly as the reviewed area191 dispatcher reads
 * them (halfword seed, two word contexts, kind 4). */
extern u16 D_80145048; /* @source 0x80145048 @kind data */
extern u32 D_80145040; /* @source 0x80145040 @kind data */
extern u32 D_80145044; /* @source 0x80145044 @kind data */

/* Shared front-end cue dispatcher at 0x8015DF18; the cue id arrives
 * zero-extended, as the other area175 lifts in this target declare it. */
void func_8015DF18(u16 arg0);

/* Shared front selector request at 0x8019FA28: it copies the seed and the two
 * context words into the shared front-selection cells, resolves the pending
 * request and sets the front-end phase byte; declaration taken from its lifted
 * body in src/bof3/ui/func_8019FA28.c. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F3F2C
 * @behavior World-overlay position callback. Both arguments are 16.16 values
 *   whose integer halves are compared against one tile rectangle: while the
 *   first argument's half lies in 0x41..0x44 and the second argument's half in
 *   0x14..0x17 the entry is inside the trigger zone, and any other position
 *   stores 1 into the latch byte 0x801455C7 and returns 0. Inside the zone the
 *   body only acts while that latch still reads nonzero: it clears the latch,
 *   queues front-end cue 0x20C through func_8015DF18 and then requests the
 *   pending front selector context through func_8019FA28, passing the shared
 *   seed halfword 0x80145048, the shared context words 0x80145040 and
 *   0x80145044 and kind 4. The function returns 0 on every path.
 *
 * Name evidence: the trigger sequence is the reviewed one from the lifted
 *   area191 dispatcher (mode 10 requests the front selector context from
 *   0x80145048 with 0x80145040/0x80145044 as kind 4 through func_8019FA28),
 *   and the cue id 0x20C is the front-end cue the world04 area-module entries
 *   of this family queue. The zone itself carries no name in the payload, so
 *   the name records only the one-shot zone-entry request and keeps the tile
 *   bounds in this description.
 *
 * Shape: the payload never names this address - no `jal` immediate and no
 *   pointer word in the payload reference it - so no in-image caller fixes the
 *   argument roles and they stay structural. The 0x90-byte span is the row's
 *   whole size, so no surviving slack is claimed. The same body sits at payload
 *   offset 0x7E4 of the world04 overlays 176 through 185, byte-identical at all
 *   36 words except the absolute `j` at span word 27, which shifts by exactly
 *   the 0xB48 offset delta between the spans; every one of those rows is still
 *   an unlifted `asm` span, so the clone lever had no source to copy and the
 *   body is seeded from this target's own bytes.
 *
 * Representation: the latch is reached through a pointer local inside the zone
 *   arm, which is what keeps its address in one register across the test and the
 *   store and frees the branch delay slot for the cue id. Two losing spellings
 *   measured on the way: reading and clearing the byte directly through its
 *   symbol left 27/36 instructions (the address was recomputed for the clear and
 *   the delay slot filled with a nop), and the same pointer hoisted above the
 *   tile tests left 30/36 (the address computation moved into the prologue and
 *   the re-arm store reused it).
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 requestFrontSelectorContextOnceOnWorldZoneEntryWorld04Area17513_801F3F2C(
    s32 arg0, s32 arg1) {
  if ((u32)((arg0 >> 16) - 0x41) < 4u &&
      (u32)((arg1 >> 16) - 0x14) < 4u) {
    u8* latch;

    latch = &D_801455C7;
    if (*latch != 0) {
      *latch = 0;
      func_8015DF18(0x20C);
      func_8019FA28(D_80145048, D_80145040, D_80145044, 4);
    }
  } else {
    D_801455C7 = 1;
  }
  return 0;
}
