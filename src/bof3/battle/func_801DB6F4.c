#include "bof3/battle/battle03_internal.h"

/* @source 0x801DB6F4
 * @behavior Returns the index of the first battler slot that func_801DB524
 * reports available, scanning downward and wrapping around the requested slot:
 * for requested indices below three it scans the local work slots from that
 * index down to zero and then from two down to that index plus one, and for
 * higher indices it scans the enemy work slots from that index down to three
 * and then from ten down to that index plus one; returns 0xFF when no slot is
 * found.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 84/84 instructions, 336 bytes, live byte match. The descending
 * counters stay `u8` with an explicit `(s8)` comparison so every test emits the
 * original `sll ...,24` sign test (`bltz`/`bgez` per direction) and the plain
 * 8-bit `andi ...,0xff` argument/return path of the sibling func_801DB5CC; the
 * second loop of each side compares the sign-extended counter against the
 * sign-extended requested index, which keeps that bound hoisted in a
 * callee-saved register exactly as the original does.
 */
u8 func_801DB6F4(u8 arg0) {
  u8 i;

  if (arg0 < 3u) {
    for (i = arg0; (s8)i >= 0; i--) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
    for (i = 2u; (s8)i > (s8)arg0; i--) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
  } else {
    for (i = arg0; (s8)i >= 3; i--) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
    for (i = 0xAu; (s8)i > (s8)arg0; i--) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
  }
  return 0xFFu;
}
