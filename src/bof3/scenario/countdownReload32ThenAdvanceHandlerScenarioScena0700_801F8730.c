#include "bof3/bof3.h"

/* @source 0x801F8730
 * @behavior Frame-countdown handler for one work-table entry: it decrements the
 * entry byte at +0x02 (unsigned byte, so the store wraps at 0xFF), returns while
 * the decremented count is nonzero, and when the count reaches zero it reloads
 * that byte with the constant 0x20 and increments the handler-index byte at
 * +0x01, advancing the entry to the next callback of its row. The entry pointer
 * is the only argument and the function returns nothing; it is a leaf with no
 * frame, and the constant is materialised in the branch delay slot of the
 * countdown test. The entry is one of the eight 0x1C-byte records of the
 * 0x800E4800 work table walked by func_801F85D8, which reads the record byte
 * +0x01 as an index into the overlay row at 0x801FDDF0 and calls the selected
 * entry with the record pointer; this function is that row's entry 1
 * (0x801F86BC / 0x801F8730 / 0x801F8764 / ...). No in-image jal targets the
 * address and the only image word naming it is its 0x801FDDF0 row slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownReload32ThenAdvanceHandlerScenarioScena0700_801F8730(u8* entry) {
  if (--entry[2] == 0) {
    entry[2] = 0x20;
    entry[1]++;
  }
}
