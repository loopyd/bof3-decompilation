#include "bof3/bof3.h"

/* @source 0x80146189 @kind unknown */
extern u8 D_80146189;

/* @source 0x80148210 @kind unknown */
extern u16* D_80148210;

/* @source 0x801F3AEC
 * @behavior Overlay handler reached from the handler table word at 0x801F4904:
 *           when the shared main-RAM byte at 0x80146189 reads 2, advances the
 *           halfword at offset 0x0A of the work record published through the
 *           global pointer cell 0x80148210 by 3. The record address is
 *           materialised once and the halfword is read and written back through
 *           that same register. Takes no arguments, returns nothing and uses no
 *           stack frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword0aBy3WhenSharedByte2World03Area14413_801F3AEC(void) {
  if (D_80146189 == 2) {
    D_80148210[5] += 3;
  }
}
