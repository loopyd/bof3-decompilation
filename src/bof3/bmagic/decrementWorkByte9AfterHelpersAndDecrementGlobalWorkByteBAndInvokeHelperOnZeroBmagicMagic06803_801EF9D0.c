#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name this overlay family's exact lifts use for the same
 * main-executable cell. The original materialises the cell with its own lui /
 * lw pair at the single access site (lui $v1,0x1F80 / lw $v1,0x44($v1)), so the
 * body keeps the cell at its source site instead of caching it in a local. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Pointer cell at 0x801EB4E0 of the main executable: the word holds the address
 * of the shared battle work record whose byte 0xB this handler decrements. It
 * sits outside this overlay image and this target's generated undefined-syms
 * map already binds the raw address form, so it needs no row in this map. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Main-executable helper at 0x801E5988, invoked after the shared record's byte
 * 0xB has been decremented. Outside this payload, so it keeps a raw
 * origin-tagged prototype and its role stays address-anchored. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* In-payload helpers this handler calls before the countdown. Their bodies are
 * still assembly rows of this target: 0x801EFBE8 opens a matrix frame and
 * derives a transform from the work record, 0x801EFA48 branches on
 * GetGraphType and emits primitives. Both are reached only through this
 * handler, take no arguments and return nothing. */
void func_801EFBE8(void);
void func_801EFA48(void);

/* @source 0x801EF9D0
 * @behavior Step handler published by the fifth word of this overlay's handler
 *           run at 0x801EFCC8: that word is entry 4 of the run the dispatcher
 *           at 0x801EF73C indexes with the work record's byte 1, and the same
 *           word is entry 3 of the run the dispatcher at 0x801EF780 indexes
 *           with the record's byte 2. No jal word in this payload targets the
 *           address. Calls the helper at 0x801EFBE8, then the one at 0x801EFA48,
 *           then PopMatrix, and decrements the work object's unsigned byte 0x9
 *           with a byte-width read-modify-write (lbu / addiu -1 / sb) tested at
 *           byte width (andi 0xFF / bnez on the stored value, empty delay slot),
 *           so the truncation supplies the compared value and the byte is not
 *           re-read. Only when that countdown reaches zero it loads the record
 *           pointer cell 0x801EB4E0 (lui %hi / lw %lo) and decrements that
 *           record's byte 0xB the same byte-width way, then invokes the helper
 *           at 0x801E5988, the store filling the call's delay slot. The 0x18
 *           byte frame keeps only $ra: it takes no arguments, returns nothing
 *           and touches no other state. Field roles stay offset-anchored. The
 *           statement order follows the exact sibling
 *           decrementWorkByte9AndHalfword3EAfterHelpersAndPublishByteBOnZeroBmagicMagic10403_801F034C
 *           for the two helper calls, the PopMatrix and the call in the zero
 *           arm, and the exact sibling
 *           decrementGlobalWorkByteBAndInvokeHelperBmagicMagic03803_801EF884
 *           for the byte-0xB decrement followed by the helper call. No
 *           byte-identical copy of this 120-byte span exists in any payload
 *           under out/binaries: a payload scan that masked every j/jal word
 *           found the span only in this target's own binary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AfterHelpersAndDecrementGlobalWorkByteBAndInvokeHelperOnZeroBmagicMagic06803_801EF9D0(void) {
    func_801EFBE8();
    func_801EFA48();
    PopMatrix();
    if (--g_battle_work[9] == 0) {
        D_801EB4E0[0xB]--;
        func_801E5988();
    }
}
