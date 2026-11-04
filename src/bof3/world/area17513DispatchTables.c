#include "bof3/emi/dispatchers.h"

extern void (*D_801F45EC[])(void); /* @source 0x801F45EC @kind data */
extern void (*D_801F45F8[])(void); /* @source 0x801F45F8 @kind data */
extern void (*D_801F460C[])(void); /* @source 0x801F460C @kind data */

/* @source 0x801F2C18
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F45EC. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table45ecWorld04Area17513_801F2C18, D_801F45EC);

/* @source 0x801F31E4
 * @behavior Reads the non-volatile scratchpad work-object pointer cell at
 * 0x1F800044, indexes the five-entry handler table D_801F45F8 at 0x801F45F8
 * with that object's dispatch byte at offset 0x01 (scaled by four) and calls
 * the selected handler; takes no arguments, returns nothing, and reads or
 * writes no other work object byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1HandlerWorld04Area17513_801F31E4, D_801F45F8);

/* @source 0x801F352C
 * @behavior Reads the non-volatile scratchpad work-object pointer cell at
 * 0x1F800044 and calls the handler pointer selected by that object's dispatch
 * byte at offset 0x01 in the overlay's four-entry handler table D_801F460C at
 * 0x801F460C (entries 0x801F3570, 0x801F35B4, 0x801F3644 and 0x801F36BC);
 * takes no arguments, returns nothing and reads or writes no other
 * work-object byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchWorkByte1Table460cWorld04Area17513_801F352C, D_801F460C);
