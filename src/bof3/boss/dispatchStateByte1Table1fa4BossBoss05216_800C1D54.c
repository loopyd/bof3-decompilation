#include "bof3/emi/dispatchers.h"

/* This overlay's shared data run starts at 0x800C1F3C and holds five packed
 * byte/word slots followed by twenty-two code pointers from 0x800C1F50 to
 * 0x800C1FA4, mixing this payload's functions with the shared main-executable
 * handlers at 0x801E.... The whole run sits inside the 0x800C1F3C `textbin`
 * row, which the Splat layout places, so it is only referenced here. This
 * dispatcher indexes it from that run's last word, 0x800C1FA4, so only index 0
 * reaches an entry inside the payload, 0x800C1D98, which is the following
 * sibling dispatcher; any higher work-record byte reads past the payload. */
extern void (*D_800C1FA4[])(void); /* @source 0x800C1FA4 @kind data */

/* @source 0x800C1D54
 * @behavior Overlay state dispatch: loads the non-volatile scratchpad work
 * pointer cell at 0x1F800044, takes that work record's dispatch byte at offset
 * 0x01 as the index and calls the handler pointer at that index of the local
 * handler table based at 0x800C1FA4. The selected handler's $v0 is left
 * untouched by the epilogue, so this shim's result is whatever that handler
 * returns. The original does not bound the index before the scaled table load,
 * and the slots after the index byte and after that load stay nops. The
 * pointer-cell load precedes frame creation; the 0x18-byte frame only keeps $ra
 * at 0x10 across the indirect call. No word of the payload names 0x800C1D54 and
 * no jal site in it targets that address, so its caller is outside this
 * overlay. Takes no arguments and forwards none. Same shape as the already-exact
 * siblings dispatchStateByte1Table2744BossBoss00816_800C1EB0 of boss008/16 and
 * dispatchStateByte1Table2d24BossBoss01816_800C1B84 of boss018/16, which read
 * the same dispatch byte through their own tables.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_1, dispatchStateByte1Table1fa4BossBoss05216_800C1D54, D_800C1FA4);
