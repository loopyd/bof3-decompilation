#include "bof3/bof3.h"

/*
 * Code-pointer table at 0x801F1BC4, the table this dispatcher indexes:
 * entry 0 = 0x801EF6AC, entry 1 = 0x801F0100, entry 2 = 0x801F0438,
 * entry 3 = 0x801F0B44, entry 4 = 0x801EF7FC, entry 5 = 0x801EFA68,
 * entry 6 = 0x801EFB64, entry 7 = 0x801EFBF8, entry 8 = 0x801EFC74,
 * entry 9 = 0x801EFEF0, entry 10 = 0x801EFF48, entry 11 = 0x801F0034,
 * entry 12 = 0x801F0178, entry 13 = 0x801F01A8, entry 14 = 0x801F01F8,
 * entry 15 = 0x801F0264, entry 16 = 0x801F02B0, entry 17 = 0x801F04C0,
 * entry 18 = 0x801F053C, entry 19 = 0x801F058C.
 */
extern void (*func_801F1BC4[])(void); /* @source 0x801F1BC4 @kind data */

/* @source 0x801EF668
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 is scaled by four and selects an entry of the
 * overlay-local code-pointer table at 0x801F1BC4 (the entry is read back with
 * lw from that table), and the selected entry is invoked with no arguments. It
 * reads no state other than that byte and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic13103_801EF668(void) {
  func_801F1BC4[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
