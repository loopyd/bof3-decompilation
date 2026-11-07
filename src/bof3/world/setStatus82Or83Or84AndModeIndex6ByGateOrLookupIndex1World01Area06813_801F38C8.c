#include "bof3/bof3.h"

extern s8 D_801490C7;
extern u16 D_801490A8;
/* @source 0x801D4285 @kind bss — shared mode index: selects the handler from
 * the D_801D41E0 table and is set to 6 when an entry action starts
 * (include/bof3/ui/sisyou00_internal.h). */
extern u8 D_801D4285;
/* Main-executable byte lookup. Every shipped caller passes a work-record byte
 * as its single index and masks the result to a byte, so the return is
 * byte-wide; the subsystem is still unknown and the callee keeps its
 * address-anchored name. */
extern u8 func_80166438(u32 index);

/* @source 0x801F38C8
 * @behavior Overlay entry handler that publishes the shared status halfword
 *           D_801490A8 and the shared mode index byte D_801D4285. While the
 *           gate byte D_801490C7 is nonzero it stores 0x84 into the status and
 *           6 into the mode index. Otherwise it asks the main-executable
 *           lookup func_80166438 for the entry count of index 1 and, when that
 *           byte is below 0xF, stores 0x83 into the status and 6 into the mode
 *           index; when the byte is 0xF or above it stores 0x82 into the
 *           status and leaves the mode index untouched. The two branches that
 *           write the mode index share one store site: the original
 *           materialises each arm's status constant in a branch delay slot and
 *           cross-jumps the identical status/mode-index store pair.
 *           Takes no arguments and returns nothing; the 0x18-byte frame only
 *           keeps $ra across the lookup call.
 *           Constant/variance evidence: the status literals 0x82/0x83/0x84,
 *           the mode-index literal 6 and the 0xF bound come from these original
 *           bytes; the 0x68-byte span 0x801F38C8..0x801F3930 has no
 *           byte-identical copy in any shipped payload. The gate read and the
 *           0x84/mode-index-6 arm follow the exact sibling
 *           setStatus84AndModeIndex6IfGateElseFFFFWorld01Area05013_801F2D04, and
 *           the shared mode index byte is documented as the D_801D41E0 handler
 *           selector set to 6 by an entry action.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatus82Or83Or84AndModeIndex6ByGateOrLookupIndex1World01Area06813_801F38C8(
    void) {
  if (D_801490C7 == 0) {
    if (func_80166438(1) < 0xF) {
      D_801490A8 = 0x83;
      D_801D4285 = 6;
    } else {
      D_801490A8 = 0x82;
    }
  } else {
    D_801490A8 = 0x84;
    D_801D4285 = 6;
  }
}
