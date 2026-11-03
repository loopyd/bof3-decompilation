#include "bof3/bof3.h"

typedef void (*BmagicMagic03803Handler)(void);

/*
 * Handler-pointer table at 0x801EF9A4, the table this dispatcher indexes:
 * entries 0..6 are 0x801EF114, 0x801EF8B8, 0x801EF1AC, 0x801EF3BC,
 * 0x801EF590, 0x801EF710 and 0x801EF884. The four packed byte words at
 * 0x801EF9C0..0x801EF9CC follow them in the same 0x34-byte run, then the
 * pointers 0x801EF8FC and 0x801EF954.
 */
extern BmagicMagic03803Handler D_801EF9A4[];

/* @source 0x801EF0D0
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its unsigned state byte at offset 1 is scaled by four and that entry of the
 * overlay-local handler-pointer table at 0x801EF9A4 (seven in-image code
 * pointers, entry 0 = 0x801EF114 through entry 6 = 0x801EF884, before the four
 * packed byte words at 0x801EF9C0) is invoked with no arguments. It reads no
 * other state and writes none; the 0x18-byte frame only keeps $ra across the
 * indirect call and the pointer-cell load precedes frame creation. No word in
 * this payload addresses 0x801EF0D0, so the caller is outside the overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic03803_801EF0D0(void) {
  D_801EF9A4[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
