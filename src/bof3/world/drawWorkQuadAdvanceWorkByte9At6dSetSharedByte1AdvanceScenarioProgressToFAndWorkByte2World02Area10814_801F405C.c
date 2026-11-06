#include "bof3/bof3.h"

/* Shared scenario progress byte at 0x80146864, bound by the shared map. This
 * step only runs while it holds 0x0E and advances it to 0x0F when the work
 * record's byte +0x09 reaches 0x6D. The body reaches it through a pointer
 * local: its address is live from the entry test to the store after the helper
 * call, which is what keeps it in $s0. */
extern u8 g_ScenarioProgress;

/* Shared overlay state byte at 0x8014933B. This step raises it to 1 and the
 * sibling handler 0x801F4128 clears it when the progress byte reads 0x1C.
 * Splat resolves the address through this target's generated undefined-symbol
 * map, which the still-unlifted sibling 0x801F4128 references. */
extern u8 D_8014933B; /* @source 0x8014933B @kind data */

/* Scratch work record pointer cell 0x1F800044, named through this target's map
 * instead of the fixed-address accessor: the original re-materialises the cell
 * address for each of its three loads (`lui $vN,0x1F80` plus the folded
 * `lw $vN,68($vN)`), while the constant-address macro folds the whole address
 * into one shared base. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* Overlay quad helper at 0x801F4160, still unlifted assembly inside this
 * target, so only its prototype is declared. It fills an FT4 primitive from
 * the scratchpad coordinate block at 0x1F800014, projects the four vertices
 * through RotTransPers4, folds its second argument's 5-bit page field into the
 * texture-page word it submits, and appends the 0x28-byte primitive. */
void func_801F4160(s32 width, s32 page);

/* @source 0x801F405C
 * @behavior Overlay step handler, entry 1 of the four-pointer handler table at
 *           0x801F5718 that the byte-+0x02 dispatcher 0x801F3FF8 indexes. It
 *           runs only while the shared scenario progress byte at 0x80146864
 *           reads 0x0E: it loads the scratch work record published through
 *           pointer cell 0x1F800044, hands that record's byte +0x09 to the
 *           overlay quad helper at 0x801F4160 as both arguments (`lbu` of the
 *           byte, then one register copy into the second argument slot),
 *           re-reads the cell and advances the same byte +0x09 by one, then
 *           re-reads the cell a third time and returns while the advanced byte
 *           is still below 0x6D. Once it reads 0x6D or more it stores 1 into
 *           the shared overlay state byte at 0x8014933B, advances the progress
 *           byte to 0x0F and increments the record's dispatch byte +0x02
 *           through the pointer the third load left, which hands the next
 *           frame to entry 2 of the same table; every other progress value
 *           leaves the record untouched. The cell is reloaded for each of those
 *           three groups, the last load serving both the bound test and the
 *           dispatch-byte store, and the group before the call keeps its own
 *           cursor local so it holds $v0 while the two later groups hold $v1.
 *           The function has a 0x18-byte frame keeping the return address at
 *           +0x14 and the progress-byte address in $s0 across the helper call,
 *           takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkQuadAdvanceWorkByte9At6dSetSharedByte1AdvanceScenarioProgressToFAndWorkByte2World02Area10814_801F405C(void) {
  u8* work;
  u8* quad;
  u8* progress;

  progress = &g_ScenarioProgress;
  if (*progress != 0xE) {
    return;
  }
  quad = workRecord44World02Area10814;
  func_801F4160(quad[9], quad[9]);
  work = workRecord44World02Area10814;
  work[9]++;
  work = workRecord44World02Area10814;
  if (work[9] < 0x6D) {
    return;
  }
  D_8014933B = 1;
  *progress = 0xF;
  work[2]++;
}
