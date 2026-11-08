#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after each call of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access that follows a
 * call. The reads the body makes without an intervening store share one load,
 * and the original shows that: the arm that clears work byte 0x2B reuses the
 * register that already holds the record pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body reads only row byte 0, the occupancy
 * flag, and the original re-materialises `lui at,0x8014` and adds the scaled
 * row offset for that access, so the base keeps an address-derived symbol.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* Front-end record pointer cell of the shared main executable. The body clears
 * this record's state byte 0x12B through the published pointer, and no other
 * byte of that record is touched here, so the record role stays
 * address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared readiness helper of the shared main-executable image, outside this
 * payload. The original zero-extends the answer with `andi $v0,0xFF` before it
 * tests it, so this call site reads the result as a byte of a wider return and
 * masks it here; the helper's own byte return type is what the other exact
 * source of this target that calls it declares. This target's generated
 * undefined_funcs_auto.txt binds the address, and the remaining asm rows of
 * this payload reference it too.
 */
u32 func_8014DAEC(void);

/* Shared arrival-sequence selector of the shared main-executable image, outside
 * this payload: it takes the route index in one byte and selects that route's
 * arrival sequence. The original passes the work route byte 0x08 straight into
 * the argument register and discards any result. This target's generated
 * undefined_funcs_auto.txt binds the address, and the exact source of this
 * target that calls it declares the same prototype.
 */
u32 func_8014D8D4(u8 route);

/* @source 0x801CE92C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body address appears once in the payload bytes, as
 *           word 0x801CFACC, entry 5 of the twenty-three-entry code-pointer run
 *           based at 0x801CFAB8 that this overlay places as the textbin row at
 *           that base, and no jal of the payload names the span. It takes no
 *           arguments and returns nothing. Work byte 0x07 is the
 *           pending-request flag of the work object, the byte the sibling step
 *           0x801CE820 of this payload also clears after it queues cue 0x100
 *           plus work halfword 0x2C through func_8015DF18. While that byte is
 *           nonzero the body runs the shared readiness helper func_8014DAEC and
 *           returns while the helper reports not-ready; once it reports ready
 *           the body hands the work route byte 0x08 to the shared
 *           arrival-sequence selector func_8014D8D4 and clears the flag. While
 *           the flag reads zero the body runs the same helper and again returns
 *           while it reports not-ready; once ready, and while the 0x74-byte
 *           front-end row that work byte 0x0B selects still has its occupancy
 *           byte 0 clear, it clears work byte 0x2B and the state byte 0x12B of
 *           the front-end record published at 0x80146250. The roles of work
 *           byte 0x07, of work byte 0x0B, of work byte 0x2B, of the selected
 *           row and of that state byte stay address-anchored. Its first load
 *           precedes frame creation, the 0x18-byte frame only keeps $ra at
 *           0x10 across the calls, the flag-clear store fills the delay slot of
 *           the jump to the shared return label, and the slots after the flag
 *           byte, after the row byte, after the row occupancy byte and after
 *           both helper results stay nops.
 * Shape note: one scan of every extracted EMI payload of the tree found this
 * 0xD4-byte span byte-identical in exactly one other binary, the plp567 overlay
 * at the same address, whose splat row still read asm when this source was
 * written, so no source existed to take from it and the clone adapter was
 * declined on that measurement; the same scan showed the plp025, plp056, plp257
 * and plp457 copies differ from this span in exactly one word, word 21, the
 * absolute immediate of the jump to the shared tail label. The reviewed exact
 * lift of the plp457 copy at 0x801CEB48 seeded the body below, whose statement
 * order, row stride, helper spelling and flag-clear delay slot are the same at
 * both addresses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequenceForWorkByte8AndClearWorkByte7WhenHelperReadyElseClearWorkByte2BAndState12BWhenFrontEndRowUnoccupiedPlcharPlp57800_801CE92C(
    void) {
  if (D_1F800044[0x07] != 0) {
    if ((func_8014DAEC() & 0xFF) == 0) {
      return;
    }
    func_8014D8D4(D_1F800044[0x08]);
    D_1F800044[0x07] = 0;
  } else if ((func_8014DAEC() & 0xFF) != 0) {
    if (D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] == 0) {
      D_1F800044[0x2B] = 0;
      D_80146250[0x12B] = 0;
    }
  }
}
