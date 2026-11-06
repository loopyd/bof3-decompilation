#include "bof3/emi/dispatchers.h"

/* This overlay's handler run at 0x801F5280: code pointers into this payload, from
 * 0x801F32D8 at the base through 0x801F3960 at 0x801F52AC, followed by non-pointer
 * words. The run sits in the Splat `bin` tail segment, so it is only referenced
 * here. This dispatcher indexes it from 0x801F52A0, entry 8 of that run, so the
 * four entries it can reach are 0x801F3708, 0x801F3788, 0x801F3860 and
 * 0x801F3960. Its own address is not in the run: its only in-image reference is
 * the jal at 0x801F34E8 in the wrapper invokeTwoHelpersWorld01Area06513_801F34D8. */
extern void (*D_801F52A0[])(void);

/* @source 0x801F36C4
 * @behavior Overlay step dispatch: reads the state byte at +3 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the handler
 *           pointer at that index of the local handler table based at 0x801F52A0.
 *           The original does not bound the index before the scaled table load. It
 *           reads no other state and writes none; the record-pointer load precedes
 *           frame creation, the 0x18-byte frame only keeps $ra at 0x10 across the
 *           indirect call, and the slots after the index byte and after the table
 *           load stay nops. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_3, dispatchStateByte3Table52A0World01Area06513_801F36C4, D_801F52A0);
