#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after each call of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access that follows a call.
 * The two reads the second arm makes without an intervening call share one load,
 * and the original shows that: the arm that clears work byte 0x2B reads that byte
 * through the register that already holds the record pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body reads only row byte 0, the occupancy
 * flag, and the original re-materialises `lui at,0x8014` and adds the scaled row
 * offset for that access, so the base keeps an address-derived symbol.
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
 * masks it here; the helper's own byte return type is recorded by the other
 * overlays that call it. This target's generated undefined_funcs_auto.txt binds
 * the address, and the remaining asm rows of this payload reference it too.
 */
u32 func_8014DAEC(void);

/* Shared arrival-sequence selector of the shared main-executable image, outside
 * this payload: it takes the route index in one byte and selects that route's
 * arrival sequence. The original passes the work route byte 0x08 straight into
 * the argument register and discards any result. This target's generated
 * undefined_funcs_auto.txt binds the address, and the remaining asm rows of this
 * payload reference it too.
 */
u32 func_8014D8D4(u8 route);

/* @source 0x801CF124
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: its address appears once in the payload's bytes, as word
 *           0x801CF124 at 0x801CFF04, entry 10 of the twenty-three-entry
 *           code-pointer run based at 0x801CFEDC. The payload's own readers of
 *           that run touch entry 0 through the base itself and entries 11
 *           through 22 through the interior bases 0x801CFF08, 0x801CFF14,
 *           0x801CFF20 and 0x801CFF2C, so no instruction of this payload reads
 *           this entry. It takes no arguments and returns nothing. The work byte
 *           0x07 is the pending-request flag of the work object, the same flag
 *           claimFrontEndRecordHandler3a clears after it queues a cue: while that
 *           byte is nonzero the body runs the shared readiness helper
 *           func_8014DAEC and returns while the helper reports not-ready, and
 *           once it reports ready it hands the work route byte 0x08 to the
 *           shared arrival-sequence selector func_8014D8D4 and clears the flag.
 *           While the flag reads zero the body runs the same readiness helper and
 *           again returns while the helper reports not-ready; once ready, and
 *           while the front-end row selected by the work byte 0x0B at stride
 *           0x74 still has its occupancy byte 0 clear, it clears work byte 0x2B
 *           and the state byte 0x12B of the front-end record published at
 *           0x80146250. The roles of work byte 0x2B, of work byte 0x0B, of the
 *           selected row and of that state byte stay address-anchored. Its first
 *           load precedes frame creation, the 0x18-byte frame only keeps $ra at
 *           0x10 across the calls, the flag-clear store fills the delay slot of
 *           the jump to the shared return label, and the slots after the flag
 *           byte, after the row byte, after the row occupancy byte and after both
 *           helper results stay nops. The 0xD4-byte body is identical word for
 *           word to the span at 0x801CFE4C of the plp025 overlay, to the span at
 *           0x801CFA28 of the plp015 overlay, and to the spans at 0x801CF654 of
 *           plp056, 0x801CEB48 of plp457, 0x801CED2C of plp567 and 0x801CED2C of
 *           plp578, except for the absolute jump immediate of that shared tail
 *           label, which follows each overlay's own base address. Those seven
 *           copies are shape evidence at this overlay's address and not
 *           byte-identical clones, so this seed is written from the shape and
 *           proven here by this target's own comparison.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectArrivalSequenceForWorkByte8AndClearWorkByte7WhenHelperReadyElseClearWorkByte2BAndState12BWhenFrontEndRowUnoccupiedPlcharPlp25700_801CF124(
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
