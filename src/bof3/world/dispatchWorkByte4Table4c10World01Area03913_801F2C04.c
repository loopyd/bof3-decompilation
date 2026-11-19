#include "bof3/emi/dispatchers.h"

/* @source 0x801F4C10
 * @kind data
 */
extern void (*workByte4HandlersWorld01Area03913[])(void);

/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the handler byte at +4 of the work
 * record published at the scratchpad pointer cell 0x1F800044, scales it by
 * four and calls the entry at that index of the local handler table at
 * 0x801F4C10, whose two entries are the routines at 0x801F2C48 and
 * 0x801F2C88. Takes no arguments and returns nothing, and its 0x18-byte
 * frame only keeps the return address across the call.
 *
 * Name evidence: the only word of this payload that names 0x801F2C04 is entry
 * 0 of the overlay entry table at 0x801F4B90, which the header word at
 * 0x801F4C08 points at. The index byte at +4 selects inside this overlay's
 * own table, so the name describes the local table and claims no engine-wide
 * role.
 *
 * Shape evidence: this 17-word, 68-byte body is byte-identical across the
 * corpus to the dispatchers lifted in the same family at world01 area056
 * 0x801F3284, world02 area079 0x801F2E24, world02 area099 0x801F2D68 and
 * world04 area191 0x801F31A0. Each of those spans differs from this one in
 * the single %lo word of the load that reads its own table (payload compares
 * at offsets 0x684, 548, 360 and 1440), which is shape evidence and not clone
 * eligibility, so every binding and literal here is written from this span's
 * own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4,
                      dispatchWorkByte4Table4c10World01Area03913_801F2C04,
                      workByte4HandlersWorld01Area03913);
