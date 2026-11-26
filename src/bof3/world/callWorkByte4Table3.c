#include "bof3/emi/dispatchers.h"

/* This overlay's handler table at 0x801F3B7C: the two entry points that arm and
 * clear the work record's byte 4, the routines at 0x801F3458 and 0x801F34D0.
 * Both words are the tail of the reviewed `textbin` run based at 0x801F3554, so
 * the table is only referenced here. */
extern void (*workByte4HandlerTable[])(void); /* @source 0x801F3B7C @kind data */

/* @source 0x801F3414
 * @behavior Overlay step dispatch: the leading two instructions read the work
 *           record pointer out of the scratchpad cell 0x1F800044 (`lui
 *           v0,0x1F80`, `lw v0,0x44(v0)`) before the 0x18-byte frame is
 *           reserved, then load byte +4 of that record unsigned, scale it by
 *           four and index the local handler table based at the word
 *           0x801F3B7C. The selected entry is called through `jalr v0` with a
 *           `nop` delay slot, after which the body reloads ra from 0x10(sp),
 *           releases the frame and returns. Takes no arguments and returns
 *           nothing; the frame keeps the return address across the call.
 *           The table's two words are 0x801F3458 and 0x801F34D0, which set and
 *           clear that same record byte 4 from the shared mode byte at
 *           0x80143BB0, so byte +4 selects which of the two runs. The word at
 *           0x801F3B34 publishes this dispatcher itself, first in that run. The
 *           body reads and writes no other state, so the name describes this
 *           overlay's own table and claims no engine-wide role.
 *           Shape evidence: this 17-instruction, 0x44-byte body is the shared
 *           dispatcher shape. A payload comparison over every extracted binary
 *           found no byte-identical copy - the nearest 28 spans differ only in
 *           the table word at span offset 0x24 - so the table binding here is
 *           written from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4,
                      callWorkByte4Table3,
                      workByte4HandlerTable);
