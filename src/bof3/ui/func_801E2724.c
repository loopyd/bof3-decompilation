#include "bof3/ui/shop00_internal.h"

/* The two record selector bytes live at +0 and +0x24 of the byte pointer in
 * D_80148378. The original materializes that address once and reaches the
 * second byte with a 0x24 offset instead of a folded absolute store, so the
 * pointer is a mutable byte pointer advanced by the record stride (same shape
 * as initializeShopUiState); a plain absolute access folds back into a
 * one-instruction lui $at store. */
/* @source 0x801E2724
 * @behavior initializes the two 0x24-byte shop panel records anchored at
 *           D_80148378: writes the leading byte 1 of both records, the fixed
 *           field runs (7, 16, 2, 320, 63) for the first record and
 *           (7, 17, 2, 180, 240) for the second, and copies the main-RAM byte
 *           D_80148361 into the first record's byte at +0x0A. The caller
 *           func_801E2150 runs it inside the shop UI transition after the
 *           sound cue 0x102 and before it clears D_80148624, re-arms
 *           phaseTimer with 8 and advances the UI sub-step byte D_80148652.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2724(void) {
  u8* base;
  u8 snapshot;

  base = D_80148378;
  snapshot = D_80148361;
  D_80148379 = 7;
  D_8014837A = 16;
  D_8014837B = 2;
  D_8014837C = 320;
  D_8014837E = 63;
  D_80148382 = snapshot;
  base[0] = 1;
  D_8014839D = 7;
  D_8014839E = 17;
  D_8014839F = 2;
  D_801483A0 = 180;
  D_801483A2 = 240;
  base += 0x24;
  base[0] = 1;
}
