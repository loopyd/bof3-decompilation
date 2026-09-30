#include "bof3/bof3.h"

typedef void (*Scena0300FrameHandler)(void);

extern u8* D_1F800044;
extern Scena0300FrameHandler func_801FCF24[];

/* @source 0x801F6E70
 * @behavior Dispatches this overlay's per-frame handler from the scratchpad
 * work object: it reads the work-object pointer cell at 0x1F800044 (before the
 * frame is created), takes that object's unsigned state byte at offset 1,
 * scales it by four and calls that entry of the overlay-local handler table at
 * 0x801FCF24, whose three pointer words are 0x801F6EB4 (the overlay's
 * 0x3AC-byte setup/step routine), 0x801F7260 and 0x801F7334; the words that
 * follow them are not pointers. It reads no other state and writes none, takes
 * no arguments and returns nothing of its own: the 0x18-byte frame only keeps
 * $ra across the indirect call and the callee's $v0 is discarded. The address
 * is the overlay's first code byte (payload offset 0x270, splat boundary 624),
 * and its 68 bytes are the same instruction shape as the exact sibling
 * dispatchers dispatchWorkByte1Handler (0x801F6DC0) and
 * dispatchWorkByte1HandlerScenarioScena1500_801F6F38 (0x801F6F38), which
 * differ only in the table address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0300_801F6E70(void) {
  func_801FCF24[D_1F800044[1]]();
}
