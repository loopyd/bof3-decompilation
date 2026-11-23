#include "bof3/bof3.h"

extern u8 D_80146871;   /* @source 0x80146871 @kind data */
extern u16 D_80143F00;  /* @source 0x80143F00 @kind unknown */
extern u8 *D_80181164[]; /* @source 0x80181164 @kind table */

/* @source 0x801F2C04
 * @behavior Reads bit 0 of the shared front-state byte D_80146871. When the bit
 *           is set the body stores 2 into byte 0x0C and byte 0x14 of the world
 *           record selected by the pending world id D_80143F00 through the
 *           pointer table D_80181164, and when the bit is clear it stores 0 into
 *           those two bytes instead. Every store re-reads D_80143F00 and
 *           re-loads the record pointer, so the body makes four table lookups;
 *           the constant 2 is materialised once and reused by its arm's pair of
 *           stores, and the clear arm stores the zero register directly. Takes
 *           no arguments, calls nothing and returns nothing; the 0x10-byte frame
 *           reserves no slot.
 * @status exact
 * @match 100.00
 * @residual none
 * First seed, written directly from the original bytes rather than copied from a
 * sibling: no payload among the 398 under out/binaries/ carries this 180-byte
 * span (one `find` over the payload bytes returned only this overlay), so the
 * clone lever had no representative and no shape donor existed. Declared widths
 * follow the original accesses: an unsigned byte read of D_80146871, an unsigned
 * halfword read of the pending world id D_80143F00, and a pointer table of
 * unsigned-byte records at D_80181164 whose entries are reloaded per store, so
 * each arm spells the whole lookup expression twice instead of caching a record
 * pointer. The 2 constant is materialised once per arm because both stores of
 * that arm write it.
 */
void stateWorldFrontClear(
    void) {
  if ((D_80146871 & 1) != 0) {
    D_80181164[D_80143F00][0xC] = 2;
    D_80181164[D_80143F00][0x14] = 2;
  } else {
    D_80181164[D_80143F00][0xC] = 0;
    D_80181164[D_80143F00][0x14] = 0;
  }
}
