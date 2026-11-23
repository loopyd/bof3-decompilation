#include "bof3/bof3.h"

/* @source 0x801F2D3C
 * @behavior Overlay init helper: clears the three lead bytes (offsets 0x0, 0x1
 *           and 0x2) of each of the 0x100 entries of the 0x18-byte-stride work
 *           table at 0x800E4800, the main-RAM work area this overlay shares with
 *           the world and scenario overlays. It keeps one base for the byte at
 *           offset 0x0 and a second base two bytes higher for the bytes at 0x1
 *           and 0x2, stepping both by 0x18 and an unsigned 16-bit counter by 1
 *           in a bottom-tested loop; it takes no arguments, returns nothing and
 *           reads no other state. Its only in-payload reference is the jal at
 *           0x801F2C7C inside func_801F2C48, which publishes the three words at
 *           +0x34, +0x38 and +0x3C of the scratchpad work object at 0x1F800044
 *           from 0x80145EC4 through 0x80145ECC, calls this table reset, then
 *           initializes the 16 work records through
 *           initialize16RecordsSignedPairs and
 *           advances the work object's byte at +0x1. The 0x40-byte body is
 *           byte-identical (payload cmp of the two 64-byte spans) to the
 *           already-exact clearWorkTableLeadBytesAt800E4800ScenarioSce15ef300_801D16DC
 *           lift in emi/scenario/sce15ef3/00, which supplied this statement
 *           order, operand form and name. Byte 0 of each entry is that entry's
 *           active flag; the roles of bytes 1 and 2 stay unknown here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateTableLeadBytes(void) {
  u8* record;
  u8* lead;
  u16 i;

  record = PSX_PTR(u8, 0x800E4800u);
  i = 0u;
  lead = record + 2u;

  do {
    record[0] = 0;
    lead[-1] = 0;
    lead[0] = 0;
    lead += 0x18u;
    i += 1u;
    record += 0x18u;
  } while (i < 0x100u);
}
