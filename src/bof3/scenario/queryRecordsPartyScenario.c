#include "bof3/bof3.h"

/* @source 0x80146254 @kind unknown — live count of the local work records. */
extern u8 D_80146254;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90. */
extern u8 D_80145F09[];

/* @source 0x801FDA34
 * @behavior Scans the live local work records for one whose party byte is 2.
 * The records are the 0x140-byte stride rows based at 0x80145E90 of the shared
 * main-executable data segment, counted by the shared byte D_80146254; the
 * scan starts at row 0 and reads only the party byte at +0x79, which it
 * compares against the constant 2. It returns 1 at the first match and 0 when
 * every live row has a different party byte, takes no arguments, and writes
 * nothing. The 0x64-byte body has no frame, so the match return and the
 * exhausted-scan return share the single `jr $ra` epilogue; the count byte is
 * the loop bound rather than a local copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 queryRecordsPartyScenario(void) {
  u8 i;
  u32 offset;

  i = 0;
  if (D_80146254 != 0) {
    do {
      offset = i * 0x140;
      if (D_80145F09[offset] == 2) {
        return 1;
      }
      i++;
    } while (i < D_80146254);
  }
  return 0;
}
