#include "bof3/bof3.h"

extern int rand(void);

/* @source 0x801D2750
 * @behavior Draws a random nibble through rand() and redraws while that nibble
 *           is zero or above the exclusive limit bound + 1, so the returned
 *           value lies in 1 through bound. The bound argument is masked to 8
 *           bits, which caps the reachable range at 1 through 15. Nothing is
 *           read or written and no other call is made.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 pickRandomValueOneThroughBound(u8 bound) {
  s32 limit;
  s32 value;

  limit = bound + 1;
  do {
    value = rand() & 0xF;
  } while (value == 0 || value >= limit);
  return value;
}
