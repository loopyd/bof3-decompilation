#include "bof3/bof3.h"

typedef void (*BmagicMagic15103Handler)(void);

void func_801EEE38(void);
void func_801EF318(void);

/* @source 0x801EEDE0
 * @behavior Reads the scratchpad work pointer at 0x1F800044 and uses the
 * work object's unsigned byte at offset 2 to select a handler from a local
 * two-entry array: 0x801EEE38 followed by 0x801EF318. Invokes the selected
 * handler without arguments. The original does not check the index.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2LocalPairBmagicMagic15103_801EEDE0(void) {
  BmagicMagic15103Handler handlers[2] = { func_801EEE38, func_801EF318 };

  handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
