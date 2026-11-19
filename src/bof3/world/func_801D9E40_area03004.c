#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801D9E40
 * @behavior Dispatches shared pad-state mask D_80145AA8 & 0xD4. The 0x04 and
 * 0x80 arms select work modes 0x0A and 9; the 0x10 arm advances work
 * step 3. The 0x40 arm selects mode 9 when either shared byte
 * D_80145026 or D_80145028 is zero. Otherwise it tests the
 * matching-slot count: a passing test with D_801E3204 zero sets that
 * byte to 1 and selects work step 7. A failed test clears D_801E3204.
 * The other count-test paths select work mode 2 and clear work step 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Five arm bodies read the scratchpad cursor cell through
 * WORLD00_AREA030_SCRATCH_PTR, the constant-address volatile form the exact
 * sibling confirmSelectionStep (0x801DDDBC) already uses, while the 0x10 arm and
 * every second `work[3]` read keep the D_1F800044 symbol view. Both spellings
 * name the same volatile cell (WEAK_SYMBOL_AT(D_1F800044, 0x1F800044)). The
 * constant-address arms make the `modeByte = 7` / `D_801E3204 = 1` constant the
 * head of its block, so reorg fills the preceding branch delay slot
 * (`beq ...,0x04` / `beq ...,0x80` / `bnez D_801E3204`), which is where the
 * retained symbol-only spelling lost three instructions (+12 bytes). */
void func_801D9E40(void) {
  u8 *work;

  switch (D_80145AA8 & 0xD4) {
  case 0x10:
    modeByte = 7;
    D_8014403D = 4;
    work = D_1F800044;
    work[3]++;
    break;
  case 0x80:
    modeByte = 7;
    D_8014403D = 4;
    work = WORLD00_AREA030_SCRATCH_PTR;
    work[2] = 9;
    D_1F800044[3] = 0;
    break;
  case 0x40:
    modeByte = 7;
    if (D_80145026 == 0 || D_80145028 == 0) {
      work = WORLD00_AREA030_SCRATCH_PTR;
      D_8014403D = 4;
      work[2] = 9;
      D_1F800044[3] = 0;
    } else {
      if ((u8)hasAtLeastTwentyTwoMatchingSlotsWorld00Area03004() != 0) {
        if (D_801E3204 == 0) {
          D_801E3204 = 1;
          work = WORLD00_AREA030_SCRATCH_PTR;
          D_8014403D = 4;
          work[3] = 7;
          break;
        }
      } else {
        D_801E3204 = 0;
      }
      work = WORLD00_AREA030_SCRATCH_PTR;
      work[2] = 2;
      D_1F800044[3] = 0;
    }
    break;
  case 0x04:
    modeByte = 7;
    D_8014403D = 4;
    work = WORLD00_AREA030_SCRATCH_PTR;
    work[2] = 0xA;
    D_1F800044[3] = 0;
    break;
  }
}
