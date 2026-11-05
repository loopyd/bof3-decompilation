#include "bof3/bof3.h"

/* @source 0x801F7574
 * @behavior Clears byte 0 of each of 32 records with stride 0x28 in the
 * bank at 0x800E5000 selected by scratchpad work byte 6 (bank stride 0x500).
 * Takes no arguments and returns nothing. Other record bytes are unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearSelectedRecordBankLeadBytesScenarioScena0900_801F7574(void) {
  u8 *base = PSX_PTR(u8, 0x800E5000u);
  u8 *record;
  u8 i;

  record = base + SPAD_PTR_SLOT(u8, 0x44)[6] * 0x500;
  i = 0;
  do {
    record[0] = 0;
    i++;
    record += 0x28;
  } while (i < 0x20);
}
