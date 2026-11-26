#include "bof3/emi/dispatchers.h"

/* This overlay's handler run: the payload's reviewed `textbin` row D_800C1CB0
 * spans 0x800C1CB0 to the payload's last word, and its code pointers start at
 * 0x800C1CD8. This dispatcher indexes the run from that base's word 12, the
 * interior word 0x800C1D08, so the run stays one reviewed row and this
 * declaration binds only the word the scaled index is added to. From word 12
 * the entries are 0x801E3BAC, 0x801E3BD0, 0x801E3BAC, 0x801E4490, this
 * payload's own 0x800C18FC and the main-executable 0x801E4928.
 */
extern void (*D_800C1D08[])(void); /* @source 0x800C1D08 @kind data */

/* @source 0x800C18B8
 * @behavior Second-level overlay dispatch on the work record byte at offset 2:
 * loads the work-record pointer published at the scratchpad pointer cell
 * 0x1F800044, takes that record's unsigned byte at offset 2, scales it by four
 * and calls the matching entry of the handler run from its word 12 at
 * 0x800C1D08 with no arguments, so no argument register is loaded and the
 * callee's value in $v0 is what this function returns. The original does not
 * bound the index before the scaled table load. It reads no other state and
 * writes none; the pointer-cell load precedes frame creation, the 0x18-byte
 * frame only keeps $ra at 0x10 across the indirect call, and the slots after
 * the index byte and after the table load stay nops. It is a second-level step
 * of the same overlay: the first-level dispatcher indexes its run by the record
 * byte at offset 1, and word 6 of that run, 0x800C1CF0, is this function, so
 * that step continues here on the byte at offset 2. That word is the only
 * in-image reference to this span and the payload holds no jal naming it, so
 * the overlay stepper that walks the run is its caller. Same C shape as the
 * exact sibling dispatchStateByte2Table2064BossBoss03616_800C18B8 at this same
 * address in BIN/BOSS/BOSS036.EMI#16, whose scaled load differs only in its
 * table word.
 * @status exact
 * @match 100.00
 * @residual none
 */
EMI_DEFINE_DISPATCHER(EMI_DISPATCH_WORK_BYTE_2, callStateByte2Table6, D_800C1D08);
