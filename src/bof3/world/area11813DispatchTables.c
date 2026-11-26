#include "bof3/emi/dispatchers.h"

extern void (*D_801F5124[])(void);
extern void (*D_801F5130[])(void);
extern void (*D_801F516C[])(void);
extern void (*D_801F5184[])(void);
extern void (*D_801F519C[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F5124.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable5124World, callStateTable5124World, D_801F5124);

/* @source 0x801F2E20
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F5130.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable5130World, callStateTable5130World, D_801F5130);

/* @source 0x801F3014
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F516C.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable516World, callStateTable516World, D_801F516C);

/* @source 0x801F3C10
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F5184.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable5184World, callStateTable5184World, D_801F5184);

/* @source 0x801F410C
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F519C.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(callStateTable519World, callStateTable519World, D_801F519C);
