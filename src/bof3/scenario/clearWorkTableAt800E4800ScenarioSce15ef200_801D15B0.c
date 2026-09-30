#include "bof3/bof3.h"

/* @source 0x801D15B0
 * @behavior Overlay init helper: clears the lead byte of each of the 0x10
 * entries of the 6-byte-stride record table at 0x800E4800 (the main-RAM work
 * table base the world and scenario overlays walk as well), stepping the entry
 * pointer by 6 and an unsigned byte counter by 1 in a bottom-tested loop; it
 * takes no arguments, returns nothing and reads no other state. The only
 * in-payload caller is the step handler func_801D0DE8 (jal at 0x801D0E00),
 * entry 0 of that overlay's handler table at 0x801D1B2C, which stores 0x12C to
 * the work object's halfword at +0x2E and zeroes its byte at +0x9 first.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableAt800E4800ScenarioSce15ef200_801D15B0(void) {
  u8* work;
  u8 i;

  work = PSX_PTR(u8, 0x800E4800u);
  i = 0u;

  do {
    *work = 0;
    work += 0x6u;
    i += 1u;
  } while (i < 0x10u);
}
