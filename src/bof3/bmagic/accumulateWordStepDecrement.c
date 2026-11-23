#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map. The original materializes the cell address once at entry and reads the
 * work record through that single loaded pointer, so every field access below
 * is an offset of one local. This overlay's lifts
 * clearWorkBytesFive and
 * decrementCoordWordsTwice
 * read the same cell and had measured exact when this seed was written. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF5D4
 * @behavior Width-ramp step body, word 1 of the three-word handler run the
 *           payload header holds at 0x801EEC1C, between 0x801EF454 and
 *           0x801EF61C. The word at 0x801EEC20 is the only word in this
 *           payload that names this address and no jal reaches it, so the
 *           running overlay selects it through work byte 0x02. It reads the
 *           scratchpad work pointer cell at 0x1F800044 once and, within that
 *           record, adds the word at 0x18 to the word at 0x0C, then decrements
 *           the word at 0x18 and stores the decremented value, which is the
 *           value tested against 0x10, so the store fills the branch delay
 *           slot. Some handler in this same overlay draws the work record with
 *           the word at 0x0C as its effect radius, and the 0x801EF454 seed
 *           leaves both words at 0x10 before selecting the 0x801EF61C handler
 *           that accumulates the same pair, so the word at 0x18 reads as the
 *           per-frame growth step this body walks down to 16. When the
 *           decremented step lands exactly on 0x10, work byte 0x02 is
 *           incremented, retiring this body and selecting the 0x801EF61C
 *           handler for the next frame. The addition uses the word at 0x18 as
 *           read before the decrement, and that load is issued twice because
 *           the store to the word at 0x0C precedes the second read. Leaf with
 *           no frame: it saves no register, takes no argument, returns no
 *           value, and reads or writes no state outside the work record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void accumulateWordStepDecrement(void) {
  u8 *work;

  work = g_battle_work;
  *(u32 *)(work + 0x0C) += *(u32 *)(work + 0x18);
  if (--*(u32 *)(work + 0x18) == 0x10) {
    work[0x02]++;
  }
}
