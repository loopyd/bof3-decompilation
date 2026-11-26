#include "bof3/bof3.h"

/* Scratchpad pointer cell 0x1F800044, which publishes the active 0x98-byte
 * work record. Declared as a named symbol rather than through SPAD_PTR_SLOT so
 * each access re-materialises the cell address with its own lui, which is what
 * the original does before and after the two calls; the exact target siblings
 * stateByteAdvanceOverlay
 * and clearWorkByte48Submit declare the same
 * cell this way. */
extern u8* scratchpadWorkRecord; /* @source 0x1F800044 @kind data */

/* Leading words of the 0x140-byte-stride party action record array in main RAM,
 * outside this overlay's image. The exact ui lift func_8019EB38 publishes
 * 0x80145EC4 and 0x80145EC8 as the party coordinate pair, and the exact bmagic
 * lift seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238
 * copies words 0, 4 and 8 of the same array into work words 0x34, 0x38 and 0x3C.
 * The role of the third word is not established beyond that copy, so the three
 * cells stay address-anchored; Splat binds them through this target's generated
 * undefined_syms_auto.txt. */
extern u32 D_80145EC4; /* @source 0x80145EC4 @kind unknown */
extern u32 D_80145EC8; /* @source 0x80145EC8 @kind unknown */
extern u32 D_80145ECC; /* @source 0x80145ECC @kind unknown */

/* @source 0x801F2D3C @kind function */
extern void stateTableLeadBytes(void);
/* @source 0x801F2FE0 @kind function */
extern void initialize16RecordsSignedPairs(void);

/* @source 0x801F2C48
 * @behavior Handler entry 0 of the work-byte-1 dispatch table at 0x801F4308,
 *           the table the dispatcher at 0x801F2C04 indexes with byte +1 of the
 *           work record published through the scratchpad pointer cell
 *           0x1F800044. It stores the shared word triplet at 0x80145EC4 through
 *           0x80145ECC into work words 0x34, 0x38 and 0x3C, resets the
 *           main-RAM work table at 0x800E4800 through
 *           stateTableLeadBytes,
 *           initializes the 16 work records through
 *           initialize16RecordsSignedPairs,
 *           then re-reads the pointer cell and adds one to dispatch state byte
 *           +1, so the next step runs entry 1 at 0x801F2CB4. It takes no
 *           arguments and returns nothing; the frame only keeps $ra across the
 *           two calls. Its 108-byte body differs from the 0x801D0C48 init in
 *           emi/scenario/sce15ef3/00 at the two jal immediates and at the three
 *           instructions that sibling adds between the last store and the
 *           re-read of the pointer cell, so that span supplies the statement
 *           order, not byte identity.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateCoordsInitializeRecords(void) {
  u8* work;

  work = scratchpadWorkRecord;
  *(u32*)(work + 0x34) = D_80145EC4;
  *(u32*)(work + 0x38) = D_80145EC8;
  *(u32*)(work + 0x3C) = D_80145ECC;
  stateTableLeadBytes();
  initialize16RecordsSignedPairs();
  work = scratchpadWorkRecord;
  work[1]++;
}
