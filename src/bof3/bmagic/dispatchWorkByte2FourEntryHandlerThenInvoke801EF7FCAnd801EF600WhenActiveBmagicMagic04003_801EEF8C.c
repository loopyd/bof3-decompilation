#include "bof3/bof3.h"

typedef void (*BmagicMagic04003Byte2Handler)(void);
typedef struct BmagicMagic04003Byte2DispatchTable {
  BmagicMagic04003Byte2Handler handlers[4];
} BmagicMagic04003Byte2DispatchTable;

extern BmagicMagic04003Byte2DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801EF7FC(void); /* @source 0x801EF7FC @kind function */
extern void func_801EF600(u8 arg0, u16 arg1); /* @source 0x801EF600 @kind function */

/* @source 0x801EEF8C
 * @behavior Copies the four handler pointers at 0x801EEC10 into the frame and
 * invokes the entry indexed by scratchpad work byte 2 without a bounds check.
 * The entry is published at payload offset 0x106C. After dispatch, reloads the
 * work cell and, if byte 0 bit 0 and byte 2 are set, calls 0x801EF7FC, then
 * calls 0x801EF600 with byte 9 and three times byte 0xA, and again with byte 9
 * plus four narrowed to a byte and twice byte 0xA. Each direct call is followed
 * by a fresh work-cell load before the next work-byte reads. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2FourEntryHandlerThenInvoke801EF7FCAnd801EF600WhenActiveBmagicMagic04003_801EEF8C(void) {
  BmagicMagic04003Byte2DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC10;
  handlers.handlers[g_battle_work[2]]();
  work = g_battle_work;
  if ((work[0] & 1u) && work[2] != 0u) {
    func_801EF7FC();
    func_801EF600(g_battle_work[9], g_battle_work[10] * 3);
    func_801EF600((u8)(g_battle_work[9] + 4), g_battle_work[10] * 2);
  }
}
