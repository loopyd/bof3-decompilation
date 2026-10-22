#include "bof3/ui/game00_internal.h"

/**
 * @source 0x8019FF88
 * @behavior publishes a pending frontend request: a nonzero kind first sets bit
 * 3 of the world-flags halfword D_8014625A, then the active flag D_80146260,
 * the handler index kind (D_80146261), a cleared byte D_80146262 and the two
 * argument words D_80146264/D_80146268 are stored; while the record count
 * D_80146254 is nonzero it walks that many 0x140-byte-stride records at
 * D_80145FBB, copying the per-kind byte D_801C8380[kind] into each record's
 * leading byte and setting bit 1 of each record's second byte at D_80145FBC.
 * @status exact
 * @match 100.00
 * @residual none
 * Matching shape: the D_80145FBC pointer local has to be published before the
 * flag store, otherwise the record-count guard is allocated $v0 after the store
 * constant; the D_801C8380[kind] load stays inside the loop and the record
 * stores keep the explicit byte-offset local.
 */
void func_8019FF88(u8 kind, u32 context_a, u32 context_b) {
  u8* flags;
  s32 i;

  flags = D_80145FBC;
  if (kind != 0) {
    D_8014625A |= 8;
  }
  D_80146260 = 1;
  D_80146261 = kind;
  D_80146262 = 0;
  D_80146264 = context_a;
  D_80146268 = context_b;

  for (i = 0; i < D_80146254; i++) {
    s32 offset;

    offset = i * 0x140;
    D_80145FBB[offset] = D_801C8380[kind];
    flags[offset] |= 2;
  }
}
