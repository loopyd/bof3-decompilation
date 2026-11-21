#include "bof3/bof3.h"

/* Local drawing routine of this overlay, re-declared here because this file
 * is the only one that reaches it from this handler. */
void drawMirroredWorkRecordQuadPairForGraphTypeWorld04Area16913_801F30E8(void);

/* @source 0x801F2F0C
 * @behavior Per-frame byte-2 handler of the overlay work record published at
 *           the scratchpad pointer cell 0x1F800044, reached through the
 *           handler run based at 0x801F3A90: it is word 2 of that run, whose
 *           five words are 0x801F2C48, 0x801F2DCC, this address, 0x801F2F68
 *           and 0x801F3078, and the overlay dispatcher at 0x801F2C04 calls
 *           the word the record state byte at 0x02 selects, so no jal names
 *           it. Adds 0x10 to the record's unsigned halfword at 0x30, stores
 *           the truncated halfword, and compares the value that store wrote,
 *           read back as a signed halfword: once it reaches 0xC0 the record
 *           state byte at 0x02 advances by one, so the next dispatch takes
 *           entry 3. It then calls the payload routine at 0x801F30E8, which
 *           re-reads the same record and emits its mirrored textured quad
 *           pair under the graph-type draw mode. The cell is loaded once and
 *           that one base serves both the halfword update and the byte store.
 *           Takes no arguments and returns nothing; its 0x18-byte frame only
 *           holds $ra at 0x10 across the call, whose delay slot stays empty,
 *           and the slots after the halfword load and after the branch stay
 *           nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHalfword30By10AndAdvanceState2AtC0ThenDrawWorld04Area16913_801F2F0C(
    void) {
  u8* record;
  s16* half;

  record = SPAD_PTR_SLOT(u8, 0x44u);
  half = (s16*)(record + 0x30);
  *half = (s16)(*half + 0x10);
  if (*half >= 0xC0) {
    record[2]++;
  }
  drawMirroredWorkRecordQuadPairForGraphTypeWorld04Area16913_801F30E8();
}
