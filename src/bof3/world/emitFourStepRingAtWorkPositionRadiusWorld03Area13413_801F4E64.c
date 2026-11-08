#include "bof3/bof3.h"

/* @source 0x801F5848 @kind table
 * Four signed ring x offsets (1, -1, -1, 1) stepped with the four ring angles
 * by this target's ring dispatcher; the same table shape as the area026 pair
 * D_801F33FC/D_801F340C that the already-exact sibling func_801F2D5C steps.
 */
extern s32 D_801F5848[];
/* @source 0x801F5858 @kind table
 * Four signed ring y offsets (1, 1, -1, -1) paired with D_801F5848. */
extern s32 D_801F5858[];

/* @source 0x801AFE18
 * Shared projection setup that consumes a 0x20-byte stack matrix; no argument
 * register other than a0 is live at the call site.
 */
extern void func_801AFE18(void* arg0);

/* @source 0x801F4F0C
 * This target's overlay ring emitter, still an asm boundary: it projects the
 * centre triple, offsets the first point by the radius along the passed 12-bit
 * angle and draws one semi-transparent primitive through the shared primitive
 * cursor, taking the two ring offsets as its fourth and fifth arguments.
 */
extern void func_801F4F0C(void* center, s32 radius, s32 angle, s32 xofs,
                         s32 yofs);

/* @source 0x801F4E64
 * @behavior Ring emitter for one work record: first prepares a 0x20-byte
 * projection scratch area through the shared setup at 0x801AFE18, then runs the
 * four-step ring dispatch. For each of the four steps it hands the record's
 * centre triple to the local emitter func_801F4F0C together with the passed
 * radius, the step angle, and the two signed ring offset tables: the step angle
 * is the step byte shifted by ten and clipped to 0xFC00, so the four steps sit
 * a quarter turn apart in the 12-bit angle space, while D_801F5848/D_801F5858
 * supply the matching (1, -1, -1, 1) and (1, 1, -1, -1) offset pairs. Takes the
 * record centre block and one value and returns nothing; the frame is 0x50
 * bytes holding the 0x20-byte scratch at 0x18 and $s0-$s4 with $ra above it.
 *
 * Name evidence: there is no jal caller - the only reference to 0x801F4E64 in
 * the payload is the jal at 0x801F4E5C inside work-byte-1 handler entry 5
 * (0x801F4624), which passes the record block at +0x0C and the word at +0x1C,
 * and the callee's own four-step loop over quarter-turn angles with the signed
 * offset pairs is the ring shape the already-exact area026 sibling
 * func_801F2D5C documents as its four ring steps. Nothing in the payload names
 * the record's +0x1C word beyond its role as the emitter's radius-scale
 * argument, so its role beyond "radius" stays described by offset.
 *
 * Shape evidence: world00/area026/13 holds this span at 0x801F2D5C (file offset
 * 0x15C); the two spans differ in exactly three words - the two addiu table
 * bases D_801F33FC/D_801F340C there against D_801F5848/D_801F5858 here and the
 * emitter jal (0x801F2E04 there, 0x801F4F0C here) - so it is shape evidence,
 * never clone eligibility. That sibling's exact source
 * src/bof3/world/func_801F2D5C_area02613.c supplies the statement order, the
 * 0x20-byte scratch declaration, the u8 step byte and the
 * (i << 10) & 0xFC00 angle spelling; this function's own callees, tables and
 * frame come from its original bytes. No payload among the 396 binaries under
 * out/binaries carries a byte-identical copy of this 0xA8-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void emitFourStepRingAtWorkPositionRadiusWorld03Area13413_801F4E64(
    void* center, s32 radius) {
  u8 scratch[0x20];
  u8 i;

  func_801AFE18(scratch);

  i = 0;
  do {
    func_801F4F0C(center, radius, (i << 10) & 0xFC00, D_801F5848[i],
                  D_801F5858[i]);
    i++;
  } while (i < 4);
}
