#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after each call of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-cell form and evaluates it again at every access that follows a call.
 * The two accesses the body makes with no call between them share one load, and
 * the original shows that: the arm that clears work byte 0x2B reuses the
 * register that already holds the record pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows whose byte 0 is the occupancy flag, 1 once a seeder claims the row and 0
 * while the row is still free. This body reads only that flag, and the original
 * re-materialises `lui at,0x8014` and adds the scaled row offset for the read,
 * so the base keeps an address-derived symbol.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];

/* Front-end record pointer cell of the shared main executable. The body clears
 * this record's state byte 0x12B through the published pointer and touches no
 * other byte of it, so the record role stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared readiness helper of the shared main-executable image, outside this
 * payload, answering an unsigned byte that both polling arms of this body test
 * against zero. The original zero-extends the answer with `andi $v0,0xFF`
 * before each test, so the byte return type is this target's own declaration of
 * it, which the exact sibling step 0x801CF130 of this payload also uses. Splat
 * binds the address through this target's generated undefined_funcs_auto.txt.
 * @source 0x8014DAEC
 */
extern u8 func_8014DAEC(void);

/* Shared arrival-sequence selector of the shared main-executable image, outside
 * this payload: it takes the route index in one byte and selects that route's
 * arrival sequence. The original passes the work route byte 0x08 straight into
 * the argument register and discards any result. This target's exact lifts
 * declare the same prototype, and Splat binds the address through this target's
 * generated undefined_funcs_auto.txt.
 */
u32 func_8014D8D4(u8 arg0);

/* Seeded from the lift of the same body at 0x801CE92C of plp578/00, which had
 * gated exact when this source was written. A comparison of the two payload
 * spans through `cmp` over their 0xD4 bytes proves them equal word for word
 * (md5 d3f8823cf3b3a9ed210e1fb1fd3de6f3 of both), so the statement order, the
 * row stride, the reload-per-access pointer-cell form, the helper spelling and
 * the flag-clear delay slot are that shared body, carried over with this
 * target's own callee declarations. The same scan found the plp025, plp056,
 * plp257 and plp457 copies differing from this span in exactly one word, word
 * 21, the absolute immediate of the jump to each copy's own shared tail label,
 * which is the offset delta between the two spans and so the same body again,
 * not a different callee.
 */
/* @source 0x801CE92C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: no jal of the payload names the span, and the payload's
 *           only word for the body address is 0x801D07A0, entry 5 of the
 *           twenty-four-word code-pointer run that starts at 0x801D078C (the
 *           textbin row at that base) and index 2 of the run based at
 *           0x801D0798 that this target's exact byte-2 dispatcher 0x801CE610
 *           indexes with work byte 0x02. Its two dispatchers therefore reach
 *           the body while work byte 2 reads 5 and 2 respectively, and the
 *           overlay loader that steps the outer run is its caller. It takes no
 *           arguments and returns nothing. Work byte 0x07 is the flag the
 *           preceding step 0x801CE820 of this payload clears after it queues
 *           cue 0x100 plus work halfword 0x2C, and here it gates whether the
 *           arrival sequence for work route byte 0x08 has been selected yet:
 *           while it is nonzero the body runs the shared readiness helper
 *           func_8014DAEC and returns while the helper reports not-ready, and
 *           once the helper reports ready it hands the route byte 0x08 to the
 *           shared arrival-sequence selector func_8014D8D4 and clears the flag.
 *           While the flag reads zero the body runs the same helper and again
 *           returns while the helper reports not-ready; once ready, and while
 *           the 0x74-byte front-end row that work byte 0x0B selects still has
 *           its occupancy byte 0 clear, it clears work byte 0x2B and the state
 *           byte 0x12B of the front-end record published at 0x80146250. The
 *           roles of work byte 0x0B, of work byte 0x2B, of the selected row and
 *           of that state byte in this overlay stay address-anchored. Its first
 *           cell load precedes frame creation, the 0x18-byte frame only keeps
 *           $ra at 0x10 across the calls, the cell is re-read after each call
 *           and once more for the flag clear, the flag-clear store fills the
 *           delay slot of the jump to the shared return label, and the slots
 *           after the flag byte, after the route byte, after the row byte,
 *           after the row occupancy byte and after both helper results stay
 *           nops.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequenceForWorkByte8AndClearWorkByte7WhenHelperReadyElseClearWorkByte2BAndState12BWhenFrontEndRowUnoccupiedPlcharPlp56700_801CE92C(
    void) {
  if (D_1F800044[0x07] != 0) {
    if (func_8014DAEC() == 0) {
      return;
    }
    func_8014D8D4(D_1F800044[0x08]);
    D_1F800044[0x07] = 0;
  } else if (func_8014DAEC() != 0) {
    if (D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] == 0) {
      D_1F800044[0x2B] = 0;
      D_80146250[0x12B] = 0;
    }
  }
}
