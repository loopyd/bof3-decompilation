#include "bof3/bof3.h"

/* @source 0x80146254 @kind unknown — shared live count of the 0x140-byte-stride
 * local work records. */
extern u8 D_80146254;
/* @source 0x80145F09 @kind unknown — +0x79 byte of the 0x140-byte-stride local
 * work records based at 0x80145E90. */
extern u8 D_80145F09[];

/* Three candidate values compared against the +0x79 record byte.
 * @source 0x801F5D64 @kind table
 */
extern u8 workByte79CandidateTableWorld02Area10313_801F5D64[];

/* @source 0x801F56B0
 * @behavior Overlay query: walks the three candidate bytes of the table at
 *           0x801F5D64 and, for each candidate, scans the D_80146254 live
 *           0x140-byte-stride work records for one whose +0x79 byte equals it.
 *           It returns the index of the candidate that matched, or 0xFF when no
 *           live record carries any of the three candidates. The record count is
 *           read once for the shared entry test and re-read for each candidate
 *           pass. Takes no arguments, uses no stack frame and makes no calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 findWorkByte79CandidateIndexWorld02Area10313_801F56B0(void) {
  u8 i;
  u8 j;
  u32 offset;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < D_80146254; j++) {
      offset = j * 0x140;
      if (workByte79CandidateTableWorld02Area10313_801F5D64[i] ==
          D_80145F09[offset]) {
        return i;
      }
    }
  }
  return 0xFF;
}
