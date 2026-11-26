#include "bof3/bof3.h"
#include "bof3/emi/dispatchers.h"

typedef void (*ScenarioScena0500WorkByte1Handler)(void);

/* Overlay-local handler table continuation at 0x801FD5B4, indexed by the work
 * object's unsigned byte at offset 1. It is entry 3 of the six-word in-image
 * code-pointer run that starts at 0x801FD5A8: that run's words are 0x801F743C,
 * 0x801F745C, 0x801F74CC, 0x801F7530, 0x801F7554 and 0x801F7640, so this base
 * holds the last three of them (entry 0 = 0x801F7530, entry 1 = 0x801F7554,
 * entry 2 = 0x801F7640) and the word after them at 0x801FD5C0 is 0x1C301C1C,
 * which is not a code address. The run stays one Splat row; this continuation
 * binds at its own interior word and adds no second map row.
 * @source 0x801FD5B4 @kind table
 */
extern ScenarioScena0500WorkByte1Handler D_801FD5B4[];

/* @source 0x801F74EC
 * @behavior Per-frame handler dispatcher of this overlay's second step: it reads
 * the work-object pointer cell at scratchpad 0x1F800044 before the frame is
 * created, takes that object's unsigned byte at offset 1, scales it by four and
 * calls that entry of the overlay-local handler table continuation at
 * 0x801FD5B4, so the work byte selects which of the three handlers ending the
 * overlay's six-word work-byte-1 run executes this frame. It reads no other
 * state and writes none, takes no arguments and returns nothing of its own, and
 * its 0x18-byte frame only keeps $ra across the indirect call while the
 * handler's $v0 is discarded. No jal and no word inside this payload names
 * 0x801F74EC; the only word that stores it lies outside the overlay, at
 * 0x801C7D0C in the data blob of the concurrently loaded etc/game overlay
 * (offset 0x3250C of its payload, load address 0x80195800), immediately after
 * the sibling dispatcher address 0x801F73F8 at 0x801C7D08, so the installer of
 * both entry points sits in that overlay and this function's argument roles stay
 * structural. Its 68 bytes matched the seventeen words of the exact sibling
 * dispatcher at 0x801F73F8 except the table-load immediate at offset 0x24,
 * 0xD5A8 there against 0xD5B4 here, because each dispatches through its own base
 * of the same run.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callSecondStepHandler,
                      callSecondStepHandler,
                      D_801FD5B4);
