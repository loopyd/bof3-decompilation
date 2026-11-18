#include "bof3/bof3.h"

/* Shared work-area reset clearWorkFlags in emi/etc/game/00: zeroes work-object
 * bytes 0x00-0x04 and so returns the overlay to work state 0. */
void func_80196070(void);

/* Band draw of this overlay. On its asm row when this was written, the body
 * walks the entry-count table at 0x801181D4/0x801181D5/0x801181D7 indexed by the
 * scratch halfword at 0x36 and emits one tinted quad per selected column through
 * RotTransPers, writing its first argument into the primitive colour bytes
 * 0x04-0x06 of each packet; the second argument is overwritten before any use. */
extern void func_80117E78(s32 shade, s32 arg1); /* @source 0x80117E78 */

/* Shared per-frame counter at 0x80143E6C in main RAM, the cell the composed map
 * already binds; bit 0 is the frame parity this tree masks for every-other-frame
 * work.
 * @source 0x80143E6C @kind data
 */
extern u32 D_80143E6C;

/* Scratchpad work-object pointer cell. The original materialises the cell
 * address with `lui` plus an offset load once per use, and a call to the shared
 * reset can republish the cell, so the source reads the named symbol once per
 * group.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x80117DF0
 * @behavior State-12 handler of the thirteen in-image pointers the Splat textbin
 * row D_8011818C places: word 12 at 0x801181BC, the last entry of the run. On a
 * frame whose shared per-frame counter 0x80143E6C has bit 0 clear it increments
 * the work object's byte at offset 9, the byte this overlay's earlier handlers
 * arm as a countdown, and here ramps upward instead. Once that byte reads 0x80
 * or more it resets the work object through the shared func_80196070
 * (clearWorkFlags in emi/etc/game/00), which zeroes work-object bytes 0x00-0x04
 * and so returns the overlay to work state 0. Every frame, taken or not, it then
 * runs the overlay band draw func_80117E78 with 0x80 minus that byte and a zero
 * second argument, so the bands it emits fade out as the byte climbs toward
 * 0x80. The counter word is read before the frame is created, the 0x18-byte
 * frame only keeps $ra at 0x10 across the two calls, and the slots after each
 * pointer-cell load and after the countdown byte load stay nops. No call or
 * pointer word in the payload names this function, so it is an overlay entry
 * rather than an in-payload callee. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceScratchByte9OnEvenFramesResetWorkAreaAt80ThenDrawBandWithComplementShadeWorld01Area04913_80117DF0(
    void) {
  if ((D_80143E6C & 1) == 0) {
    D_1F800044[9]++;
  }

  if (D_1F800044[9] >= 0x80u) {
    func_80196070();
  }

  func_80117E78(0x80 - D_1F800044[9], 0);
}
