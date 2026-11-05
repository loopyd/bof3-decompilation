#include "bof3/bof3.h"

typedef void (*BmagicMagic01500Handler)(void);

typedef struct BmagicMagic01500DispatchTable {
  BmagicMagic01500Handler handlers[4];
} BmagicMagic01500DispatchTable;

extern BmagicMagic01500DispatchTable workByte1HandlerTableBmagicMagic01500_801EEC14; /* @source 0x801EEC14 @kind data */

/* @source 0x801EF248
 * @behavior Copies the four handler pointers at 0x801EEC14 into a local table,
 * then calls the entry selected by unsigned byte 1 of the work record held
 * in scratchpad cell 0x1F800044. The entries are 0x801EF2B4, 0x801EF594,
 * 0x801EF610 and 0x801EF200. The indirect call sets no argument registers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic01500_801EF248(void) {
  BmagicMagic01500DispatchTable handlers;

  handlers = workByte1HandlerTableBmagicMagic01500_801EEC14;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
