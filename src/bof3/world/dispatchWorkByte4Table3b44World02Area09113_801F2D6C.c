#include "bof3/emi/dispatchers.h"

/* @source 0x801F3B44
 * @kind data
 */
extern void (*workByte4HandlersWorld02Area09113[])(void);

/* @source 0x801F2D6C
 * @behavior Overlay step dispatch: the leading two instructions read the work
 *           record pointer out of the scratchpad cell 0x1F800044 (`lui
 *           v0,0x1F80`, `lw v0,0x44(v0)`) before the 0x18-byte frame is
 *           reserved, then load byte +4 of that record unsigned, scale it by
 *           four and index the local handler table based at the word
 *           0x801F3B44. The selected entry is called through `jalr v0` with a
 *           `nop` delay slot, after which the body reloads ra from 0x10(sp),
 *           releases the frame and returns. Takes no arguments and returns
 *           nothing; the frame keeps the return address across the call. The
 *           table's leading entries are the neighbouring routines at
 *           0x801F2DB0, 0x801F2E6C, 0x801F312C, 0x801F3228, 0x801F3318 and
 *           0x801F3504, and the word at 0x801F3B5C publishes this dispatcher
 *           itself, seventh in the run.
 *           Role evidence: record byte +4 is the handler index of this
 *           overlay's palette-stage sequence, the byte the area091 sibling
 *           copyHalvedPaletteRow
 *           increments when its colour row wraps. The body indexes nothing
 *           else, so the name describes this overlay's own table and claims no
 *           engine-wide role.
 *           Shape evidence: this 17-instruction, 0x44-byte body is
 *           byte-identical across the corpus to the already-exact dispatchers
 *           at world02 area099 0x801F2D68 and world01 area039 0x801F2C04.
 *           Each of those spans differs from this one in a single word, the
 *           one at payload offset 0x24 that carries the low half of its own
 *           table (0x443b228c here, against 0x8849228c and 0x104c228c in a
 *           payload comparison), so their spans are shape evidence and not
 *           clone eligibility; every binding here is written from this span's
 *           own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_4,
                      dispatchWorkByte4Table3b44World02Area09113_801F2D6C,
                      workByte4HandlersWorld02Area09113);
