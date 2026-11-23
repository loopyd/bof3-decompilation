#include "bof3/bof3.h"

/* Shared record pointer cell at 0x1F800044. This body holds one loaded record
 * pointer across its countdown test and its first triplet store, then re-reads
 * the cell before each of the two remaining stores, so the original carries a
 * lui/lw pair per re-read.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

void func_801F2ED8(void);
void setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC(void);

/* @source 0x801F2D70
 * @behavior Entry 2 of this overlay byte-1 handler run based at 0x801F3D2C,
 *           the eight-word pointer table whose words are 0x801F2C48, 0x801F2CB4,
 *           0x801F2D70, 0x801F2E2C, 0x801F2EB0, 0x801F30E0, 0x801F35A0 and
 *           0x801F36B4 in that order and which the dispatcher
 *           callStateTable3World02 indexes with
 *           the work byte at +0x01 of the record published at the scratchpad
 *           pointer cell 0x1F800044, so this body is the step that dispatcher
 *           takes while that byte reads 2. It decrements the record countdown
 *           byte +0x09 through its own read of the cell, reads the countdown
 *           back through a second read and, while the decremented byte is
 *           nonzero, lowers the tile colour triplet at +0x5D, +0x5E and +0x5F
 *           by 4, re-reading the cell before the +0x5E and +0x5F stores while
 *           the +0x5D store goes through the pointer the read already holds.
 *           When the decremented byte reads zero it re-arms that byte with 0x0C
 *           through the same held pointer, reads the cell once more and stores
 *           3 into the dispatch byte +0x01, which hands the record to entry 3
 *           (0x801F2E2C) on the next dispatch. Both arms then call the
 *           full-screen tile draw at 0x801F2ED8, which reads the triplet as the
 *           tile colour bytes of its 0x140 by 0xF0 tile, and the work-record
 *           service at 0x801F2FCC in that order and return. Takes no arguments
 *           and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementCountdownByte9LowerWorkTripletBy4OrRearmByte9To12AndArmDispatchByte1To3World02Area08513_801F2D70(
    void) {
  u8* record;

  D_1F800044[9]--;
  record = D_1F800044;
  if (record[9] != 0) {
    record[0x5D] -= 4;
    record = D_1F800044;
    record[0x5E] -= 4;
    record = D_1F800044;
    record[0x5F] -= 4;
  } else {
    record[9] = 0xC;
    record = D_1F800044;
    record[1] = 3;
  }

  func_801F2ED8();
  setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC();
}
