#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by this overlay's dispatchers and
 * handlers. It is a named cell symbol, not the bare constant scratchpad base:
 * the original re-materialises the cell address with its own lui before every
 * one of the five loads (lui $vN,0x1F80 / lw $vN,68($vN)), so each access needs
 * its own cell load. The exact sibling tickCountdownAndClearTwoRecordBytes
 * ScenarioScena0300_801F7260 declares the same cell this way. */
extern u8* D_1F800044;

/* Shared 0x98-byte-stride work-record table in main RAM, outside this overlay's
 * image. The exact sibling tickCountdownAndClearTwoRecordBytes
 * ScenarioScena0300_801F7260 stores through the same base with the same
 * index*0x98 offset form. Splat binds the address through this target's
 * generated undefined_syms_auto.txt, so no target-map row is needed. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind data */

/* Byte 0 of record 7 of the same table (0x80145E90 + 7 * 0x98), the flag cell
 * the gate byte selects by default. Its role beyond "the record flag byte this
 * handler toggles" is not evidenced, so the name stays address-anchored. */
extern u8 D_801469B8[]; /* @source 0x801469B8 @kind data */

/* Byte 0 of record 5 of the same table (0x80145E90 + 5 * 0x98 == 0x80146888),
 * the base the two records selected by work bytes 3 and 4 are cleared through. */
extern u8 D_80146888[]; /* @source 0x80146888 @kind data */

/* @source 0x801F35A0
 * @behavior Work-byte-1 step handler, entry 6 of the local handler table at
 *           0x801F3D2C and entry 1 of the table at 0x801F3D40. It selects one
 *           byte-0 flag cell of the 0x98-byte-stride work records by work byte
 *           0xB -- the record at 0x801469B8 by default, the table base
 *           0x80145E90 when byte 0xB is nonzero -- and holds bit 0x40 of that
 *           cell. While work byte 9 is nonzero the bit is set and byte 9 is
 *           decremented with a byte-width read-modify-write, whose store fills
 *           the return jump's delay slot. When byte 9 is zero the bit is
 *           cleared, byte 0 of the records at 0x80146888 indexed by work bytes
 *           3 and 4 is cleared, and work byte 1 is set to 2. The scratchpad
 *           work-object pointer cell 0x1F800044 is re-read after every store
 *           that may alias it, which is what the original shows; the selected
 *           flag cell, the scaled index and each 0x98-byte offset are separate
 *           quantities. Leaf with no frame; nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setFlag40UntilWorkByte9ExpiresThenClearSelectedRecordsWorld02Area08513_801F35A0(
    void) {
  u8* flag;
  u32 offset;

  if (D_1F800044[9] != 0) {
    flag = D_1F800044[0xB] ? D_80145E90 : D_801469B8;
    flag[0] |= 0x40;
    D_1F800044[9]--;
  } else {
    flag = D_1F800044[0xB] ? D_80145E90 : D_801469B8;
    flag[0] &= ~0x40;
    offset = D_1F800044[3] * 0x98;
    D_80146888[offset] = 0;
    offset = D_1F800044[4] * 0x98;
    D_80146888[offset] = 0;
    D_1F800044[1] = 2;
  }
}
