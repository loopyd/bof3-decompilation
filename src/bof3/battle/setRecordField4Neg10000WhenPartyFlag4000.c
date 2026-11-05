#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E98C
 * @behavior Reads the active battler index byte D_80146394 and, for an index
 *   below 3 whose 0x140-stride party-record halfword at 0x80145F10 carries bit
 *   0x4000, stores -10000 in the halfword at byte +0x04 of the record pointed
 *   to by D_8014639C.record, advances the byte count at 0x801462F1 and clears
 *   that 0x4000 bit from the battler through func_800A36F0. An index below 3
 *   without the bit clears the record halfword to zero; indexes 3 and above
 *   leave the record and the count untouched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordField4Neg10000WhenPartyFlag4000(void) {
  u8* state;

  /* Non-volatile view: one address register serves both the guard read and the
   * reloaded call argument, and the cast below re-reads through it. */
  state = (u8*)&D_80146394;
  if (*state < 3) {
    if ((*(u16*)(D_80145F10 + *state * 0x140) & 0x4000) != 0) {
      D_8014639C.record[2] = -10000;
      /* Single-execution scope around the real counter update: its LOOP_END
       * note flushes the pending memory lists, which keeps the argument reload
       * after the 0x801462F1 store rather than in the counter load's delay
       * slot. Moving the bare increment back into the block loses that order. */
      do {
        D_801462F1++;
      } while (0);
      func_800A36F0(*(volatile u8*)state, 0x4000);
    } else {
      D_8014639C.record[2] = 0;
    }
  }
}
