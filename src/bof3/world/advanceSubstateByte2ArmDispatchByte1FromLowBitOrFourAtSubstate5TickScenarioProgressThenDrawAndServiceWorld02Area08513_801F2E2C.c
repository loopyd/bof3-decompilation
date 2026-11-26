#include "bof3/bof3.h"

/* Shared record pointer cell at 0x1F800044. This body re-reads it once after
 * its sub-state increment, so the original issues a lui/lw pair for the
 * increment and a second pair for the read-and-arm that follows.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Scenario progress byte of the main executable. The target composes
 * config/targets/shared/symbols.txt, which already binds 0x80146864, so this
 * declaration reuses that row and adds no target-map entry. The original
 * materialises the cell address once into $v0 with lui/addiu and then reads and
 * writes it at offset 0 through that base, which is the form a pointer local
 * takes; a direct symbol reference emits a %hi/%lo pair per access instead.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;

void func_801F2ED8(void);
void setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC(void);

/* @source 0x801F2E2C
 * @behavior Entry 3 of this overlay's byte-1 handler run based at 0x801F3D2C,
 *           the eight-word pointer table whose words are 0x801F2C48, 0x801F2CB4,
 *           0x801F2D70, 0x801F2E2C, 0x801F2EB0, 0x801F30E0, 0x801F35A0 and
 *           0x801F36B4 in that order and which the dispatcher
 *           dispatchStateByte1Table3 indexes with
 *           the work byte at +0x01 of the record published at the scratchpad
 *           pointer cell 0x1F800044. It increments the record sub-state byte
 *           +0x02 and reads the incremented byte back through a second load of
 *           the cell. When that byte reads 5 it increments the scenario progress
 *           byte at 0x80146864 and arms the dispatch byte +0x01 with 4, which
 *           hands the record to entry 4 (0x801F2EB0) on the next dispatch; every
 *           other value arms that byte with (value & 1) + 1, that is 1 for an
 *           even sub-state and 2 for an odd one. It then calls the full-screen
 *           tile draw at 0x801F2ED8 and the work-record service at 0x801F2FCC in
 *           that order and returns. Takes no arguments and returns nothing.
 *           Constant/variance evidence: every literal in the 0x84-byte body comes
 *           from these original bytes -- the scratchpad cell 0x1F800044, the
 *           increment 1, the sub-state gate 5, the low-bit mask 1, the bracket 1
 *           on the parity arm, the scenario progress cell 0x80146864 and the
 *           armed dispatch value 4. The compare masks the reloaded byte with 0xFF
 *           and the gate arm is the one placed out of line, so the body is
 *           written with the parity arm as the taken branch fallthrough.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceSubstateByte2ArmDispatchByte1FromLowBitOrFourAtSubstate5TickScenarioProgressThenDrawAndServiceWorld02Area08513_801F2E2C(
    void) {
  u8* record;
  u8* progress;
  u8 substate;

  D_1F800044[2]++;
  record = D_1F800044;
  substate = record[2];
  if (substate != 5) {
    record[1] = (substate & 1) + 1;
  } else {
    progress = &g_ScenarioProgress;
    (*progress)++;
    record[1] = 4;
  }

  func_801F2ED8();
  setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC();
}
