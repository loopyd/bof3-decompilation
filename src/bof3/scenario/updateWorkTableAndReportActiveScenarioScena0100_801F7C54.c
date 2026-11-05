#include "bof3/bof3.h"

void func_801F7A24(u8* record);

/* @source 0x801F7C54
 * @behavior Walks 128 work records at 0x800E4800 with stride 0x14. For
 * each record whose lead byte is nonzero, subtracts the signed halfword at
 * offset 2 scaled by 256 from its word at offset 12, then calls func_801F7A24
 * with the record. Clears the lead byte when the updated signed word is below
 * word 0x3C of the current scratchpad work object. Returns 1 if any record was
 * active, including records that remain active, and 0 otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 updateWorkTableAndReportActiveScenarioScena0100_801F7C54(void) {
  s32 active;
  u8* record;
  u8 i;
  s32* value;

  active = 0;
  record = PSX_PTR(u8, 0x800E4800u);
  i = 0;
  value = PSX_PTR(s32, 0x800E480Cu);
  do {
    if (*record != 0) {
      *value -= FIELD_REF(s16, value, -10) * 256;
      func_801F7A24(record);
      if (*value < FIELD_REF(s32, SPAD_PTR_SLOT(u8, 0x44), 0x3C)) {
        *record = 0;
      }
      active = 1;
    }
    value += 5;
    i += 1;
    record += 0x14;
  } while (i < 0x80);
  return active;
}
