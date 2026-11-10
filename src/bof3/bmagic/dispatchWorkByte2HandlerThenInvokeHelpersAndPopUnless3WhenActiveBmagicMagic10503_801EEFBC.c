#include "bof3/bof3.h"

typedef void (*BmagicMagic10503Byte2Handler)(void);

typedef struct BmagicMagic10503Byte2DispatchTable {
    BmagicMagic10503Byte2Handler handlers[4];
} BmagicMagic10503Byte2DispatchTable;

extern BmagicMagic10503Byte2DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801EF2F0(void); /* @source 0x801EF2F0 @kind function */
extern void func_801EF3A0(void); /* @source 0x801EF3A0 @kind function */

/* @source 0x801EEFBC
 * @behavior Copies the four handler pointers at 0x801EEC04 into a local
 *           table and invokes the entry indexed by work byte 2 through the
 *           scratchpad pointer cell 0x1F800044, without a bounds check.
 *           Reloads the work pointer after the handler. If work byte 0 is
 *           nonzero and byte 2 is not 3, calls the overlay helpers at
 *           0x801EF2F0 and 0x801EF3A0, then PopMatrix. Takes no arguments
 *           and returns nothing. The helper roles remain address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeHelpersAndPopUnless3WhenActiveBmagicMagic10503_801EEFBC(void) {
    BmagicMagic10503Byte2DispatchTable handlers;
    u8 *work;

    handlers = D_801EEC04;
    handlers.handlers[g_battle_work[2]]();
    work = g_battle_work;
    if (work[0] != 0 && work[2] != 3) {
        func_801EF2F0();
        func_801EF3A0();
        PopMatrix();
    }
}
