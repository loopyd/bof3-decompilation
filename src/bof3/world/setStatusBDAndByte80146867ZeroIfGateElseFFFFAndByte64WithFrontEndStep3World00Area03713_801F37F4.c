#include "bof3/bof3.h"

/* Shared gate byte this overlay's script handlers branch on. It is read with
 * lb, so it is declared signed here.
 * @source 0x801490C7 @kind data */
extern s8 D_801490C7;

/* Shared status halfword the entry handlers latch, stored with sh.
 * @source 0x801490A8 @kind data */
extern u16 D_801490A8;

/* Shared byte of the 0x80146865..0x80146867 triple that the scenario overlays
 * clear together; this handler stores it with one sb and never reads it, so
 * its role beyond that triple stays address-anchored in the name.
 * @source 0x80146867 @kind data */
extern u8 D_80146867;

/* Shared front-end step helper at 0x801BE1B0, selected by a step number. */
void func_801BE1B0(u32 step);

/* @source 0x801F37F4
 * @behavior Loads the shared gate byte D_801490C7 with lb before it grows a
 * 0x18-byte frame for its one call and branches on that byte. While the gate
 * byte reads zero it stores 0xFFFF into the shared status halfword D_801490A8,
 * then 0x64 into the shared byte at 0x80146867, then runs the shared front-end
 * step helper 0x801BE1B0 with step 3. Once the gate byte is nonzero it stores
 * 0xBD into D_801490A8 and 0 into 0x80146867 and calls nothing. Both arms
 * return through the single epilogue that reloads ra from 0x10(sp) and adds
 * 0x18 back to sp, and both store their load-address high half through $at.
 * Takes no arguments and returns nothing. No jal in this payload reaches this
 * address; its only in-payload reference is the little-endian handler word at
 * 0x801F4A4C inside the script/data blob D_801F3B34, so the entry is a script
 * handler reached only through that table and the blob carries no prototype,
 * which keeps the argument roles structural.
 *
 * Shape: written from the original bytes, whose arm order this span fixes
 *   itself - the gate-clear arm is the fall-through of the bnez, so the source
 *   tests the zero case first, and its three statements run in the order the
 *   words show: status halfword store, byte store, then the call with 3 in the
 *   jal delay slot. Because the gate byte is read once, above the frame
 *   adjustment, the condition cannot be re-tested per store, so both cells are
 *   stored inside their own arm rather than converged after the join.
 *
 * Name evidence: the two latched values and the front-end step are the whole
 *   difference between the arms, so the name carries them in the grammar this
 *   tree already uses for the same latch-then-branch handlers
 *   (setStatusF7AndByte801F294C4IfGateElseFFFFAndByte0World04Area17513_801F3DDC
 *   for the set-arm-first form, setStatusFFFFAndAreaStatus0AOr14ByGateWithFrontEndStep1Or2World01Area04113_801F3A48
 *   for the 0x801BE1B0 step call on a gate handler with a 0x18-byte frame, and
 *   this payload's own setStatus9CAndModeIndex6IfGateElseFFFFWorld00Area03713_801F376C
 *   family for the D_801490C7/D_801490A8 pair). The byte at 0x80146867 has no
 *   evidenced role of its own here, so the name keeps the address instead of
 *   borrowing a role word.
 *
 * Clone eligibility: declined. The 96-byte span was searched over the 398
 *   extracted payloads under out/binaries and byte-identical bytes occurred
 *   once only, at its own offset 0xBF4 of this payload, so no already-lifted
 *   exact sibling existed to clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setStatusBDAndByte80146867ZeroIfGateElseFFFFAndByte64WithFrontEndStep3World00Area03713_801F37F4(
    void) {
  if (D_801490C7 == 0) {
    D_801490A8 = 0xFFFF;
    D_80146867 = 0x64;
    func_801BE1B0(3);
  } else {
    D_801490A8 = 0xBD;
    D_80146867 = 0;
  }
}
