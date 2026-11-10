#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This body materialises the cell once and
 * reads three of the record's words through that single load; the record's
 * 0x34/0x38 words are the coordinate pair and 0x3C is the view-scale word the
 * sibling step at 0x801F44C4 also publishes. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Record published through the global pointer cell at 0x80148210. This body
 * reads its byte 4; the sibling step at 0x801F44C4 reads its halfword 0x0A and
 * seeds its byte 1 through the same cell, so the cell is declared at byte
 * width here where only a byte is read. */
extern u8* D_80148210; /* @source 0x80148210 @kind unknown */

/* Shared view-origin word pair: the main-loop view update at 0x801C5874 reads
 * this pair as the current view origin and subtracts the targets 0x80149308 and
 * 0x8014930C from it. */
extern s32 D_80143F7C; /* @source 0x80143F7C @kind data */
extern s32 D_80143F80; /* @source 0x80143F80 @kind data */

/* Committed view-scale word: the same view update adds the delta word at
 * 0x80143F5C to it and hands its high halfword to func_80154FD8, and the
 * scenario step at 0x801FB204 recommits it as 0x5800000. */
extern u32 D_80143F84; /* @source 0x80143F84 @kind data */

/* View-mode byte: both the shared view routine at 0x801C5DFC and the view
 * update at 0x801C5874 use it as the index into the byte table at 0x80181BD4
 * that scales the view step. */
extern u8 D_80143FC0; /* @source 0x80143FC0 @kind data */

/* View-offset term byte: func_801C5DFC multiplies it into the view offset it
 * adds to the origin pair, and func_801C5874 tests it before rebasing
 * D_80143F84 through func_80154FD8. */
extern u8 D_80143FC3; /* @source 0x80143FC3 @kind data */

/* Shared view routine. It takes a byte mode selector (it compares the byte
 * against 2 and 6 to pick the 0x8014932E view step 4, else 8), derives the view
 * offset from the D_80181BD4 table entry of the D_80143FC0 mode byte scaled by
 * the 0x80181AC0/0x80181AC4 word pair and by D_80143FC3, adds it to the two
 * D_80143F7C/D_80143F80 origin words it also mirrors into 0x80149308 and
 * 0x8014930C, and commits the per-frame view delta at 0x80143F5C. It lives in
 * the main executable payload and only its address is established, so it keeps
 * its address-qualified name. */
extern void func_801C5DFC(u8 mode);

/* @source 0x801F3C78
 * @behavior Overlay handler of this payload. Only its address is established as
 *           a handler: the word at 0x801F4910 of the local pointer run based at
 *           0x801F48F4 holds it, between the addresses of the neighbouring
 *           spans 0x801F48F4-0x801F491C, and nothing in the payload calls it.
 *           It reads the scratchpad work record through the pointer cell
 *           0x1F800044 once, before its 0x18-byte frame keeps $ra at 0x10, and
 *           copies three of that record's words out without changing them: the
 *           0x34 word into the shared view-origin word 0x80143F7C, the 0x38
 *           word into 0x80143F80, and the 0x3C word into the committed
 *           view-scale word 0x80143F84. It then copies byte 4 of the record
 *           published at 0x80148210 into the view-mode byte 0x80143FC0, writes
 *           1 into the view-offset term byte 0x80143FC3, and calls the shared
 *           view routine at 0x801C5DFC with the constant mode 3. It takes no
 *           arguments, returns nothing, and reads or writes no other state.
 *
 *           A byte search of every payload under out/binaries for this 0x6C-byte
 *           span returned exactly one hit, this span, with each jal immediate
 *           masked; the three absolute view-cell words in the body are data
 *           immediates, so the span has no twin and the clone lever was
 *           declined. The span has a frame and a jr $ra at 0x801F3CDC, and this
 *           target's own data-candidate ranking lists no range, so the data and
 *           carve levers did not apply. Statement order, the single cell load,
 *           the cast-and-offset record accesses and the declaration style come
 *           from the exact sibling step at 0x801F44C4 of this same overlay
 *           family, which publishes the same 0x3C word; the frame, the mode
 *           byte copy and the constant-3 view call are this span's own.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkViewTripletAndViewModeThenRecomputeViewForMode3World03Area14413_801F3C78(void) {
  D_80143F7C = *(s32*)(D_1F800044 + 0x34);
  D_80143F80 = *(s32*)(D_1F800044 + 0x38);
  D_80143F84 = *(u32*)(D_1F800044 + 0x3c);
  D_80143FC0 = D_80148210[4];
  D_80143FC3 = 1;
  func_801C5DFC(3);
}
