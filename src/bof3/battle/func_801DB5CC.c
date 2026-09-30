#include "bof3/battle/battle03_internal.h"

/* @source 0x801DB5CC
 * @behavior Returns the index of the first battler slot that func_801DB524
 * reports available, rotating the scan around the requested slot: for requested
 * indices below 3 it scans the local work slots from that index up to 2 and
 * then from 0 up to that index, and for higher indices it scans the enemy work
 * slots from that index up to 10 and then from 3 up to that index; returns
 * 0xFF when no slot is found.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 74/74 instructions, 296 bytes, live byte match. The u8 loop
 * counter keeps its zero-extending `andi ...,0xff` per use (compare, argument,
 * return) exactly as the original does, so the counter stays a plain u8.
 */
u8 func_801DB5CC(u8 arg0) {
  u8 i;

  if (arg0 < 3u) {
    for (i = arg0; i < 3u; i++) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
    for (i = 0; i < arg0; i++) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
  } else {
    for (i = arg0; i < 0xBu; i++) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
    for (i = 3; i < arg0; i++) {
      if (func_801DB524(i) == 0u) {
        return i;
      }
    }
  }
  return 0xFFu;
}
