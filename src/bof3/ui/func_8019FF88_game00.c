#include "bof3/ui/game00_internal.h"

/* @behavior publishes a pending frontend request: a nonzero kind first sets bit
 * 3 of the world-flags halfword D_8014625A, then the active flag D_80146260,
 * the handler index kind (D_80146261), a cleared byte D_80146262 and the two
 * argument words D_80146264/D_80146268 are stored; while the record count
 * D_80146254 is nonzero it walks that many 0x140-byte-stride records at
 * D_80145FBB, copying the per-kind byte D_801C8380[kind] into each record's
 * leading byte and setting bit 1 of each record's second byte at D_80145FBC.
 * @source 0x8019FF88
 * @status partial
 * @match 87.50
 * @residual first mismatch +0x0000: the required entry guard's record count is
 * allocated $v0 instead of the original $v1 (it cannot be hoisted above the
 * `li v0,1` store constant), which also leaves the masked kind in $v1 instead of
 * $t0 and moves `move a1,<kind>` accordingly; 42/48 instructions, size already
 * exact (192 bytes), and prologue, head store sequence, loop preheader, whole
 * loop body and epilogue are byte-identical, so the residual is confined to the
 * two head pseudos' register choice.
 * Live loop is byte-identical: the D_801C8380[kind] load must stay in the loop
 * (a `const` table declaration makes GCC 2.7.2 hoist it as loop-invariant and
 * costs 8 bytes), the D_80145FBB store must use the folded `hi(sym)+offset`
 * form (needs the explicit byte-offset local), and the D_80145FBC read/modify/
 * write must go through a pointer local (the register-based store is what blocks
 * the invariant load hoist). A `for`/`while` loop, i=0/flags statement order,
 * and a block- or function-scope `offset` all give the same head allocation.
 */
void func_8019FF88(u8 kind, u32 context_a, u32 context_b) {
  u8* flags;
  s32 i;

  if (kind != 0) {
    D_8014625A |= 8;
  }
  D_80146260 = 1;
  D_80146261 = kind;
  D_80146262 = 0;
  D_80146264 = context_a;
  D_80146268 = context_b;

  flags = D_80145FBC;
  for (i = 0; i < D_80146254; i++) {
    s32 offset;

    offset = i * 0x140;
    D_80145FBB[offset] = D_801C8380[kind];
    flags[offset] |= 2;
  }
}
