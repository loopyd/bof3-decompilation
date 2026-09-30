#include "bof3/ui/shop00_internal.h"

/* @source 0x801E0F78
 * @behavior Rebuilds the shop list/state: clears the counter byte D_801E6258
 *           and refills the eight slots of the overlay list D_801E6264 with
 *           the indices of the 0xA4-byte field records at D_8014496F whose low
 *           bit is set, then rewrites the shop UI state block (D_80148332 then
 *           D_80148333, the head pair D_80148331/D_80148330 through a pointer
 *           anchored at D_80148331 and stepped down by one, then
 *           D_80148334..D_8014862E) and, when the byte result of
 *           func_80166140(0xF) is zero, the second state block at
 *           D_80148408..0x8014840E. Called by the phase advance handler
 *           func_801DF9B4 with no arguments.
 * @status partial
 * @match 92.66
 * @residual 101/109 insns (436 bytes) match; the first live difference is the head state-block store order, where the original emits sb 0(v0) for D_80148331 before the D_80148332/D_80148333 stores while this spelling emits it after them, plus the -0x14 pair landing in v0 instead of a0 and its two schedule knock-ons
 */
/* Residual detail (one lever per live diff; the loop, prologue, epilogue and
 * every other store are exact, and the size/frame/instruction count match):
 *   - the head pair must go through a base register: the pointer needs a
 *     non-address use, so `base -= 1;` keeps the register live and the compiler
 *     folds the decrement into the -1 store offset (lui+addiu v0 = 0x80148331,
 *     sb 0(v0)/sb -1(v0), matching the original);
 *   - the D_80148332/D_80148333 stores are ordered before the base store here
 *     because that is what fixes the constant allocation: with the store order
 *     the original emits, 7 and 2 land in s1/s0 instead of s0/s1 (93/109);
 *     both spellings are reordered independent statements, no aid is used;
 *   - tried and rejected: plain &D_80148331 / D_80148330+1 / D_80148330[1] /
 *     (u32)-cast / FIELD_REF / integer-address forms (all fold the pair to
 *     per-store %hi/%lo), a two-pointer formulation, all four stores through
 *     the base, and declaring the constants as local variables (that one moved
 *     the first difference to the function head);
 *   - a volatile pointee reproduces the base register but is not evidence-backed
 *     for this plain-RAM state block, so it is not retained;
 *   - next opt-in rung: a per-object compiler/cse probe (flag-search or
 *     compiler-variants) for the remaining allocator tie-break, which is not
 *     authorized for this mission.
 */
void func_801E0F78(void) {
  u8* base;
  u8  i;
  u32 offset;

  D_801E6258 = 0;
  for (i = 0; i < 8; i++) {
    offset = i * 0xA4;
    if ((D_8014496F[offset] & 1) != 0) {
      D_801E6264[D_801E6258++] = i;
    }
  }
  base = &D_80148331;
  D_80148332 = 0;
  D_80148333 = 2;
  base[0] = 7;
  base -= 1;
  base[0] = 1;
  D_80148334 = 0x14;
  D_8014835F = 0xFF;
  D_80148358 = 0x70;
  D_80148336 = -0x14;
  D_8014835A = -0x14;
  D_8014833A = 0;
  D_80148355 = 7;
  D_80148356 = 2;
  D_80148357 = 2;
  D_80148354 = 1;
  D_8014835E = 4;
  D_80148625 = 7;
  D_80148626 = 3;
  D_8014862E = 4;
  D_80148624 = 0;
  if ((u8)func_80166140(0xF) == 0) {
    D_8014840A = 0x12;
    D_8014840C = 0x140;
    D_80148409 = 7;
    D_8014840B = 2;
    D_80148408 = 1;
    D_8014840E = 0x26;
  }
}
