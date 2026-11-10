#include "bof3/bof3.h"

typedef void (*BmagicMagic07403Handler)(void);

typedef struct BmagicMagic07403DispatchTable {
  BmagicMagic07403Handler handlers[6];
} BmagicMagic07403DispatchTable;

extern BmagicMagic07403DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEFE0
 * @behavior Copies the six handler pointers at 0x801EEC04 into its own
 *           0x30-byte frame and invokes the copied entry selected by byte 2
 *           of the work record published at scratchpad pointer cell
 *           0x1F800044. The table entries are 0x801EF058, 0x801EF2C8,
 *           0x801EF3C0, 0x801EF4D0, 0x801EF518 and 0x801EF6A4. Takes no
 *           arguments and returns nothing; no state is written directly.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic07403_801EEFE0(void) {
  BmagicMagic07403DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
