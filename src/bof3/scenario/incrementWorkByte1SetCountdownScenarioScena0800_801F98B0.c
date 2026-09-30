#include "bof3/bof3.h"

/* @source 0x801F98B0
 * @behavior Writes the countdown seed 0x3C into the scratchpad work byte at
 * pointer-slot 0x44 + 0x9 and then advances the work byte at 0x44 + 0x1 by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte1SetCountdownScenarioScena0800_801F98B0(void) {
  u32 slot_offset;

  slot_offset = 0x44u;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[9] = 0x3C;
  PSX_REF(u8*, SPAD_BASE + slot_offset)[1]++;
}
