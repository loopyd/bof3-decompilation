#include "bof3/bof3.h"

extern void (*D_801F5744[])(void);
extern void (*D_801F57A4[])(void);
extern void (*D_801F5804[])(void);
extern void (*D_801F581C[])(void);
extern void (*D_801F5828[])(void);
extern void (*D_801F5838[])(void);

/* Emit one overlay step dispatcher: call the handler that the work-record state
 * byte at the named slot selects from a local handler table. Put each emitted
 * function's metadata block directly above its standalone invocation. */
#define WORLD_DISPATCH_STATE_ONE(name, table)                                  \
  void name(void) {                                                            \
    table[SPAD_PTR_SLOT(u8, 0x44)[1]]();                                       \
  }


/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the
 *           handler pointer at that index of the local handler table based at
 *           0x801F5744. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table5744World03Area13413_801F2C04, D_801F5744);

/* @source 0x801F3560
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work
 * record published at the scratchpad pointer cell 0x1F800044 and calls the
 * handler pointer at that byte index of the local handler table based at
 * 0x801F57A4 (whose entries are overlay handler addresses 0x801F35A4 and up).
 * Same shape as the already-exact sibling
 * dispatchStateByte1Table6894World03Area12114_801F45E0, and the structural
 * twin of this target's first boundary 0x801F2C04, which dispatches the same
 * state byte through the table based at 0x801F5744. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table57a4World03Area13413_801F3560, D_801F57A4);

/* @source 0x801F3E0C
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the local handler table based at
 * 0x801F5804. That table's row in the shipped image reads, in order,
 * 0x801F3E50, 0x801F3E8C, 0x801F3F78, 0x801F40C8 and 0x801F41B0 followed by a
 * zero word at 0x801F5818 (raw .word row inside the trailing asm blob of this
 * overlay). The already-exact same-target sibling
 * dispatchStateByte1Table57a4World03Area13413_801F3560 is the same dispatch
 * through the same state byte and the table based at 0x801F57A4. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table5804World03Area13413_801F3E0C, D_801F5804);

/* @source 0x801F41D0
 * @behavior Overlay step dispatch: reads the work-record state byte at offset
 * 0x01 of the record published at the scratchpad pointer slot 0x44 and calls
 * the handler pointer at that byte index of the second local handler table
 * based at 0x801F581C. That table's row in the shipped image reads, in order,
 * 0x801F4214, 0x801F4288, 0x801F4310, 0x801F4388, 0x801F4400, 0x801F44C0,
 * 0x801F453C, 0x801F45A0, 0x801F4624, 0x801F46AC and 0x801F46CC, and the word
 * after it at 0x801F5848 is 1 rather than a code pointer (raw .word row inside
 * the trailing asm blob of this overlay); three of those entries are the
 * already-exact same-target handlers
 * clearWorkFlagsWhenNoCallbackRanWorld03Area13413_801F4310,
 * invokeHelperWorld03Area13413_801F453C and
 * invokeHelperWorld03Area13413_801F46CC. The sibling at 0x801F3E0C is the same
 * dispatch through the first local handler table 0x801F5804, and the
 * already-exact sibling dispatchStateByte1Table57a4World03Area13413_801F3560 is
 * the same dispatch through the table based at 0x801F57A4. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table581cWorld03Area13413_801F41D0, D_801F581C);

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
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table5828World03Area13413_801F4344, D_801F5828);

/* @source 0x801F455C
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record
 *           published at the scratchpad pointer cell 0x1F800044 and calls the
 *           handler pointer at that index of the local handler table based at
 *           0x801F5838. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
WORLD_DISPATCH_STATE_ONE(dispatchStateByte1Table5838World03Area13413_801F455C, D_801F5838);
