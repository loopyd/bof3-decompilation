#include "bof3/bof3.h"

/*
 * Overlay handler table at 0x801F1BEC, the table this dispatcher indexes:
 * entry 0 = 0x801F0294, entry 1 = 0x801F0568, entry 2 = 0x801F0964,
 * entry 3 = 0x801F0DA8, entry 4 = 0x801F1294, entry 5 = 0x801F1708. The scalar
 * words that follow it (0x801F1C04 and 0x801F1C08) are read by 0x801F0E54.
 */
extern void (*D_801F1BEC[])(void); /* @source 0x801F1BEC @kind data */

/* @source 0x801F0250
 * @behavior Dispatches one frame of the overlay's work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local handler table at 0x801F1BEC (six entries: 0x801F0294,
 * 0x801F0568, 0x801F0964, 0x801F0DA8, 0x801F1294, 0x801F1708), and that entry
 * is invoked with no arguments. It reads no state other than that byte and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00800_801F0250(void) {
  D_801F1BEC[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
