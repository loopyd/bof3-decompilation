#include "bof3/emi/dispatchers.h"

/* Pairs copied into work words 0x0C and 0x10 by 0x801EEC80 and 0x801EEEC0.
 * @source 0x801F08A8 @kind table */
extern u32 workSeedPairTable[][2];

extern void (*D_801F08E8[])(void);

/* @source 0x801EF09C
 * @behavior Overlay step dispatch: reads the state byte at +1 of
 *           the work record published at the scratchpad pointer cell
 *           0x1F800044 and calls the handler pointer at that index of the
 *           local handler table based at 0x801F08E8. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(dispatchStateByte1Table08, dispatchStateByte1Table08, D_801F08E8);
