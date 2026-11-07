#include "bof3/bof3.h"

/* @source 0x801F3384
 * @behavior Overlay work-table clear: it loads the base of the 0x28-byte-stride
 *           record table at 0x800E4800 as a literal address and zeroes the lead
 *           byte of its first 0x10 entries, walking the table with an unsigned
 *           byte counter in a bottom-tested loop whose pointer advance is
 *           scheduled into the back edge's delay slot. It takes no arguments,
 *           returns nothing, reads no state and calls nothing, so its 44 bytes
 *           carry no prologue and no frame. Every caller in this payload is the
 *           overlay's own init path: func_801F2C48, the first word of the
 *           handler run selected by dispatchStateByte1Table36ccWorld03Area12513_801F2C04,
 *           calls it at 0x801F2C50 and afterwards advances byte 0x1 of the work
 *           object published at the scratchpad pointer cell 0x1F800044. It is
 *           the overlay's own entry count: the same table is walked with stride
 *           0x28 by clearWorkTableAt800E4800ScenarioScena0800_801F7DE8 (0x40
 *           entries in its target) and with a 0x80-entry claim-byte sweep by
 *           clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC, so the
 *           span keeps this target's address and boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800World03Area12513_801F3384(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x28u;
    i += 1u;
  } while (i < 0x10u);
}
