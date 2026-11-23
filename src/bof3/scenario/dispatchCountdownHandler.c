#include "bof3/bof3.h"

typedef void (*Scena0300WorkByte1Handler)(void);

extern u8* D_1F800044;
/* @source 0x801FCF70 @kind data */
extern Scena0300WorkByte1Handler countdownHandlerTable[];

/* @source 0x801F7A9C
 * @behavior Reads the work-object pointer at scratchpad cell 0x1F800044,
 * before the frame is created, and dispatches through the overlay-local table
 * based at 0x801FCF70 using that object's unsigned byte at offset 1 scaled by
 * four. The word at 0x801FCF70 heads the overlay's contiguous 37-word
 * code-pointer run 0x801FCF70 through 0x801FD004; its first two words are
 * 0x801F7AE0 and 0x801F7B10, the countdown arm and countdown tick handlers of
 * this same work byte. Takes no arguments, performs no stores of its own,
 * discards the handler result, and requires a valid index; its 0x18-byte frame
 * only keeps $ra across the indirect call. Its 68 bytes are the same
 * instruction stream as the exact sibling dispatchers
 * dispatchFrameHandler (table 0x801FCF24),
 * dispatchSpreadHandler (0x801FCF50),
 * dispatchClaimHandler (0x801FCF88) and
 * dispatchEndTickHandler (0x801FCF94), differing
 * from 0x801F7DF0 in the one word that carries the table's low address,
 * 0x8C22CF70 against 0x8C22CF88. No word of this payload holds 0x801F7A9C and
 * no jal targets it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchCountdownHandler(void) {
  countdownHandlerTable[D_1F800044[1]]();
}
