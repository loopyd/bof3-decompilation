#include "bof3/bof3.h"

extern void (*D_801F5828[])(void);

/* @source 0x801F4344
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the third local handler table based
 * at 0x801F5828. That base is the fourth word of the pointer run that starts at
 * 0x801F581C, and in the shipped image the run reads, from 0x801F5828,
 * 0x801F4388, 0x801F4400, 0x801F44C0, 0x801F453C, 0x801F45A0, 0x801F4624,
 * 0x801F46AC and 0x801F46CC, followed by the non-pointer word 1 at 0x801F5848
 * (the tail of the same .word pointer run the exact sibling
 * dispatchStateByte1Table581cWorld03Area13413_801F41D0 documents). Three of
 * those entries are the already-exact same-target handlers
 * invokeHelperWorld03Area13413_801F453C,
 * invokeHelperWorld03Area13413_801F46AC and
 * invokeHelperWorld03Area13413_801F46CC. The exact same-target siblings
 * dispatchStateByte1Table57a4World03Area13413_801F3560,
 * dispatchStateByte1Table5804World03Area13413_801F3E0C and
 * dispatchStateByte1Table581cWorld03Area13413_801F41D0 are the same dispatch
 * through the tables based at 0x801F57A4, 0x801F5804 and 0x801F581C. Takes no
 * arguments, keeps no register, carries no frame and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte1Table5828World03Area13413_801F4344(void) {
  D_801F5828[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
