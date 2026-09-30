#include "bof3/ui/game00_internal.h"

/* @source 0x80198CAC
 * @behavior ticks the front-end clock bytes at D_80144FC0-D_80144FC3 unless
 * D_80143BB0 holds 9: D_80144FC3 wraps to zero when it reaches 30, then
 * D_80144FC2 wraps to zero when it reaches 60, then D_80144FC1 wraps to zero
 * when it reaches 60, then D_80144FC0 increments while it stays below 99 and
 * otherwise pins D_80144FC1 and D_80144FC2 to 59; the whole tick is skipped
 * while the low three bytes already read 63:3B:3B (99:59:59). It then counts
 * the two front-end countdown words at D_80145554 and D_80145558 down by one
 * tick - D_80145557/D_8014555B reload 29 when they would fall below zero,
 * D_80145556/D_8014555A and D_80145555/D_80145559 reload 59 the same way, and
 * D_80145554/D_80145558 decrement while nonzero, otherwise D_80145555/
 * D_80145556 and D_80145559/D_8014555A are cleared - skipping a word that is
 * already zero.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_80198CAC(void) {
  u8* clock;

  if (D_80143BB0 == 9u) {
    return;
  }

  if ((*(u32 *)(void *)&D_80144FC0 & 0xFFFFFFu) != 0x3B3B63u) {
    if (++D_80144FC3 >= 0x1Eu) {
      D_80144FC3 = 0u;
      if (++D_80144FC2 >= 0x3Cu) {
        D_80144FC2 = 0u;
        if (++D_80144FC1 >= 0x3Cu) {
          if (((u8 *)&D_80144FC0)[0] < 0x63u) {
            D_80144FC1 = 0u;
            ((u8 *)&D_80144FC0)[0] = ((u8 *)&D_80144FC0)[0] + 1u;
          } else {
            D_80144FC1 = 0x3Bu;
            D_80144FC2 = 0x3Bu;
          }
        }
      }
    }
  }

  clock = &D_80145554;
  if (*(u32 *)clock != 0u) {
    D_80145557 = D_80145557 - 1u;
    if ((s8)D_80145557 < 0) {
      D_80145557 = 0x1Du;
      D_80145556 = D_80145556 - 1u;
      if ((s8)D_80145556 < 0) {
        D_80145556 = 0x3Bu;
        D_80145555 = D_80145555 - 1u;
        if ((s8)D_80145555 < 0) {
          if (clock[0] != 0u) {
            D_80145555 = 0x3Bu;
            clock[0] = clock[0] - 1u;
          } else {
            D_80145555 = 0u;
            D_80145556 = 0u;
          }
        }
      }
    }
  }

  clock = &D_80145558;
  if (*(u32 *)clock != 0u) {
    D_8014555B = D_8014555B - 1u;
    if ((s8)D_8014555B < 0) {
      D_8014555B = 0x1Du;
      D_8014555A = D_8014555A - 1u;
      if ((s8)D_8014555A < 0) {
        D_8014555A = 0x3Bu;
        D_80145559 = D_80145559 - 1u;
        if ((s8)D_80145559 < 0) {
          if (clock[0] != 0u) {
            D_80145559 = 0x3Bu;
            clock[0] = clock[0] - 1u;
          } else {
            D_80145559 = 0u;
            D_8014555A = 0u;
          }
        }
      }
    }
  }
}
