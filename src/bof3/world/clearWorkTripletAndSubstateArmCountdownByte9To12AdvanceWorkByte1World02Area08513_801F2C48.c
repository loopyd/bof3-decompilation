#include "bof3/bof3.h"

/* Shared record pointer cell at 0x1F800044. Every store this body makes to the
 * work record goes through the pointer it holds, so each statement re-reads
 * the cell and the compiler emits its own lui/lw pair.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

void func_801F2ED8(void);
void setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC(void);

/* @source 0x801F2C48
 * @behavior Entry 0 of this overlay's byte-1 handler run based at 0x801F3D2C,
 *           the eight-word pointer table whose words are 0x801F2C48, 0x801F2CB4,
 *           0x801F2D70, 0x801F2E2C, 0x801F2EB0, 0x801F30E0, 0x801F35A0 and
 *           0x801F36B4 in that order and which the dispatcher
 *           dispatchStateByte1Table3 indexes with
 *           the work byte at +0x01 of the record published at the scratchpad
 *           pointer cell 0x1F800044, so this body is the step that dispatcher
 *           takes while that byte reads 0. It arms the record countdown byte
 *           +0x09 with 0x0C, stores 1 into the dispatch byte +0x01, which hands
 *           the record to entry 1 (0x801F2CB4) on the next dispatch, clears the
 *           record sub-state byte +0x02 and clears the tile colour triplet at
 *           +0x5F, +0x5E and +0x5D, then calls the full-screen tile draw at
 *           0x801F2ED8, which reads that triplet as the tile colour bytes of
 *           its 0x140 by 0xF0 tile, and the work-record service at 0x801F2FCC
 *           in that order and returns. Takes no arguments and returns nothing;
 *           the record pointer is read separately for the +0x09, +0x01 and
 *           +0x02 stores, then once for the three triplet stores. The stores
 *           through each loaded pointer may alias the pointer cell itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTripletAndSubstateArmCountdownByte9To12AdvanceWorkByte1World02Area08513_801F2C48(
    void) {
  u8* record;

  D_1F800044[9] = 0xC;
  D_1F800044[1] = 1;
  D_1F800044[2] = 0;
  record = D_1F800044;
  record[0x5F] = 0;
  record[0x5E] = 0;
  record[0x5D] = 0;

  func_801F2ED8();
  setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC();
}
