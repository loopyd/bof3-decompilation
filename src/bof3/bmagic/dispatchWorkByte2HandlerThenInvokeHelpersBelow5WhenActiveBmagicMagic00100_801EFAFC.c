#include "bof3/bof3.h"

typedef void (*BmagicMagic00100Byte2Handler)(void);

typedef struct BmagicMagic00100Byte2DispatchTable {
  BmagicMagic00100Byte2Handler handlers[7];
} BmagicMagic00100Byte2DispatchTable;

extern BmagicMagic00100Byte2DispatchTable D_801EEC48; /* @source 0x801EEC48 @kind data */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

extern void func_8014D290(void); /* @source 0x8014D290 @kind function */
extern void func_801F014C(void); /* @source 0x801F014C @kind function */
extern void func_801F0238(void); /* @source 0x801F0238 @kind function */

/* @source 0x801EFAFC
 * @behavior Copies the seven handler pointers at 0x801EEC48 into a local
 * table and calls the entry selected by byte 2 of the work object read through
 * scratchpad cell 0x1F800044, without an index bound. After that call, reloads
 * the work pointer and, when bytes 0 and 2 are both nonzero, calls the shared
 * helper at 0x8014D290. It then reloads the pointer and, when byte 2 is below
 * 5, calls overlay helpers 0x801F014C and 0x801F0238 followed by PopMatrix.
 * Takes no arguments and returns nothing. The local table occupies 28 bytes
 * of its 0x38-byte frame. The helper roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeHelpersBelow5WhenActiveBmagicMagic00100_801EFAFC(void) {
  BmagicMagic00100Byte2DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC48;
  handlers.handlers[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
    if (D_1F800044[2] < 5u) {
      func_801F014C();
      func_801F0238();
      PopMatrix();
    }
  }
}
