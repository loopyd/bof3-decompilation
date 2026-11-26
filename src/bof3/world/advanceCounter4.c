#include "bof3/bof3.h"

extern s32 D_801468BC;

/* @source 0x801F2EEC
 * @behavior Increments the shared 32-bit work counter at 0x801468BC by 0x800
 *           in place and returns.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The 28-byte body is byte-identical (sha1
 * 5ac626dfa7c7f3a374ef48189225c592e29e83f6) to the span at 0x801F3EE4 in
 * emi/world01/area067/14, payload offset 0x12E4 under the same load address
 * 0x801F2C00, so that span supplied the statement order, operand forms and
 * declaration style; the two payloads compare equal word for word because the
 * counter cell sits in the shared main executable. The adjacent same-target
 * span at 0x801F2ED0 carries the same seven words with the add-immediate
 * negated, which fixes the sign of the 0x800 step here.
 */
void advanceCounter(void) {
  s32* counter;
  s32 value;

  counter = &D_801468BC;
  value = *counter;
  value += 0x800;
  *counter = value;
}
