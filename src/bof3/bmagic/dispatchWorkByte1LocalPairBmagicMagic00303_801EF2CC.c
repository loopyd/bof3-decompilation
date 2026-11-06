#include "bof3/bof3.h"

typedef void (*BmagicMagic00303Handler)(void);

void func_801EF324(void);
void func_801EF708(void);

/* @source 0x801EF2CC
 * @behavior Reads the scratchpad work pointer at 0x1F800044 and uses the
 * work object's unsigned byte at offset 1 to select a handler from a local
 * two-entry array: 0x801EF324 followed by 0x801EF708. Invokes the selected
 * handler without arguments. The original reads the work-object pointer once,
 * seeds both entries into its own 0x20-byte frame ahead of the byte load, and
 * does not check the index. It reads no other state and writes none; the frame
 * only keeps $ra across the indirect call. The 88-byte body is
 * instruction-for-instruction identical to
 * dispatchWorkByte2LocalPairBmagicMagic15103_801EEDE0 apart from the work-byte
 * offset, both handler addresses and the frame-relative table slots.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1LocalPairBmagicMagic00303_801EF2CC(void) {
  BmagicMagic00303Handler handlers[2] = { func_801EF324, func_801EF708 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
