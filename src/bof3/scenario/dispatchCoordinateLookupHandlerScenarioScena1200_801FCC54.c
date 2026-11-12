#include "bof3/bof3.h"

typedef s32 (*ScenarioScena1200LookupHandler)(void);

/* Word argument view: the resident helper consumes the low byte of each
 * coordinate, as shown by its entry masks at 0x801A7AF4 and 0x801A7AF8. */
s32 func_801A7AD8(u8 *records, s32 count, u16 coordA, u16 coordB);

/* Four five-byte lookup records at 0x801FD228, each with span count 2: key byte
 * 0x88 with coordinates 0x09 and 0x59, 0x5A and 0x5B and flags 0x87, then key
 * byte 0xBC with coordinates 0x0F and 0x3B and flags 0x01. The run ends exactly
 * where the callback table starts. The address sits inside the overlay's final
 * textbin row, so the target map does not own it; splat's generated symbol file
 * binds it.
 */
extern u8 D_801FD228[];

/* Nineteen code pointers at 0x801FD23C: 0x801FCCBC three times, then 0x801FCD58,
 * 0x801FCDE0, 0x801FCE2C, 0x801FCE70, 0x801FCEB4, 0x801FCF00, ... up to
 * 0x801FD150, followed by the zero word at 0x801FD288. Entry 0 is the raw asm
 * row func_801FCCBC, which three of the four selectable indices reach.
 */
extern ScenarioScena1200LookupHandler D_801FD23C[];

/* @source 0x801FCC54
 * @behavior Overlay record selector, and word 15 (0x801FD1B4) of the per-frame
 * handler run at 0x801FD178: it passes the four five-byte lookup records at
 * 0x801FD228 and the length 4 to the resident lookup helper func_801A7AD8
 * together with the unsigned halfwords its own argument registers hold, reads
 * that answer as a signed byte, returns -1 when it is negative and otherwise
 * calls that entry of the nineteen-word code-pointer run at 0x801FD23C with no
 * arguments, returning the handler's signed low byte. The four records select
 * only indices 0 through 3 of that run: indices 0, 1 and 2 hold 0x801FCCBC and
 * index 3 holds 0x801FCD58, so every successful lookup runs one of those two
 * bodies. It makes no stores and reads no other state; the 0x18-byte frame only
 * keeps $ra across the two calls. The only in-image reference to this address is
 * that pointer word at 0x801FD1B4, and the run holding it calls its handlers with
 * no arguments, so the two halfword coordinate values are structural rather than
 * a contractual parameter pair. The 26 instructions are the same shape as the
 * exact sibling dispatchCoordinateLookupHandlerScenarioScena0100_801FE03C
 * (0x801FE03C of emi/scenario/scena01/00) and differ only in the lookup length,
 * the record table address and the handler table address.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 dispatchCoordinateLookupHandlerScenarioScena1200_801FCC54(u16 coordA, u16 coordB) {
  s8 index;

  index = func_801A7AD8(D_801FD228, 4, coordA, coordB);
  if (index < 0) {
    return -1;
  }
  return (s8)D_801FD23C[index]();
}
