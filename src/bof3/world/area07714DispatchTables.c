#include "bof3/bof3.h"

extern void (*D_801F6250[])(void);
extern void (*D_801F6260[])(void);
extern void (*D_801F6268[])(void);
extern void (*D_801F6270[])(void);
extern void (*D_801F6284[])(void);
extern void (*D_801F62A8[])(void);
extern void (*D_801F62BC[])(void);

#define BOF3_DISPATCH_WORK_SLOT_TWO(name, table)                                  \
  void name(void) {                                                            \
    table[SPAD_PTR_SLOT(u8, 0x44)[2]]();                                       \
  }


/* @source 0x801F2C04
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record published
 *           at the scratchpad pointer cell 0x1F800044 and calls the handler pointer
 *           at that index of the local handler table based at 0x801F6250.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table6250World02Area07714_801F2C04, D_801F6250);

/* @source 0x801F3C74
 * @behavior Overlay step dispatch: reads the state byte at +1 of the work record published
 *           at the scratchpad pointer cell 0x1F800044 and calls the handler pointer
 *           at that index of the local handler table based at 0x801F6260.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table6260World02Area07714_801F3C74, D_801F6260);

/* @source 0x801F3E04
 * @behavior Overlay step dispatch: reads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044, takes that work record's dispatch byte
 *           at offset 0x01 as its index and calls the handler pointer at that
 *           index of the local handler table based at 0x801F6268. The table's
 *           shipped row begins 0x801F3E48, 0x801F3F08 (both handler boundaries
 *           of this overlay) before the row runs into the overlay's byte data
 *           at 0x801F6278. Same shape as the already-exact sibling
 *           dispatchStateByte1Table57a4World03Area13413_801F3560, and the twin
 *           of this target's 0x801F4034, which dispatches the same work byte
 *           through the table based at 0x801F6270. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table6268World02Area07714_801F3E04, D_801F6268);

/* @source 0x801F4034
 * @behavior Overlay step dispatch: reads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044, takes that work record's dispatch byte
 *           at offset 0x01 as its index and calls the handler pointer at that
 *           index of the local handler table based at 0x801F6270. The table's
 *           shipped row begins 0x801F4078, 0x801F40D4 (both handler boundaries
 *           of this overlay) and then runs into the overlay's byte data at
 *           0x801F6278. Same shape as the already-exact sibling
 *           dispatchStateByte1Table57a4World03Area13413_801F3560, and the twin
 *           of this target's 0x801F3E04, which dispatches the same work byte
 *           through the table based at 0x801F6268. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_ONE(dispatchStateByte1Table6270World02Area07714_801F4034, D_801F6270);

/* @source 0x801F4CDC
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table based
 *           at 0x801F6284. The selected handler's $v0 is left untouched by the
 *           epilogue, so this shim's result is whatever that handler returns.
 *           Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte1Table6268World02Area07714_801F3E04, which
 *           dispatches work byte +0x01 through the table based at 0x801F6268,
 *           and the twin of this target's 0x801F52F8, which dispatches the
 *           same work byte +0x02 through the table based at 0x801F62A8.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table6284World02Area07714_801F4CDC, D_801F6284);

/* @source 0x801F52F8
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 *           pointer cell at 0x1F800044 (SPAD_PTR_SLOT(u8, 0x44)), takes that
 *           work record's dispatch byte at offset 0x02 as the index and calls
 *           the handler pointer at that index of the local handler table based
 *           at 0x801F62A8. The selected handler's $v0 is left untouched by the
 *           epilogue, so this shim's result is whatever that handler returns.
 *           Takes no arguments and forwards none. Same shape as the
 *           already-exact sibling
 *           dispatchStateByte1Table6270World02Area07714_801F4034, which
 *           dispatches work byte +0x01 through the table based at 0x801F6270,
 *           and the twin of this target's 0x801F4CDC, which dispatches the
 *           same work byte +0x02 through the table based at 0x801F6284.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table62a8World02Area07714_801F52F8, D_801F62A8);

/* @source 0x801F5608
 * @behavior Overlay step dispatch: reads the state byte at +2 of the work record published
 *           at the scratchpad pointer cell 0x1F800044 and calls the handler pointer
 *           at that index of the local handler table based at 0x801F62BC.
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
BOF3_DISPATCH_WORK_SLOT_TWO(dispatchStateByte2Table62bcWorld02Area07714_801F5608, D_801F62BC);
