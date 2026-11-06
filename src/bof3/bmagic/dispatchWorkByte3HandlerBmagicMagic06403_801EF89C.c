#include "bof3/bof3.h"

typedef void (*BmagicMagic06403Handler)(void);

void func_801EF8F4(void);
void func_801F001C(void);

/* @source 0x801EF89C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned byte at offset 3 selects one of the two handler addresses seeded
 * into this function's own 0x20-byte frame (entry 0 = 0x801EF8F4, entry 1 =
 * 0x801F001C), and the selected entry is invoked with no arguments. It reads no
 * state other than that byte and writes none; the frame only keeps $ra across
 * the indirect call, whose return address load follows a nop delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte3HandlerBmagicMagic06403_801EF89C(void) {
  BmagicMagic06403Handler handlers[2] = { func_801EF8F4, func_801F001C };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[3]]();
}
