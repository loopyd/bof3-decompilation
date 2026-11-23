#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before every access of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at each access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8, allocated by the record allocator below. The two
 * bytes this body seeds keep their own address-derived symbol because the
 * original re-materialises the base and adds the scaled row offset for each
 * store: row byte 0 is the occupancy flag at 0x80143FC8 and row byte 5 is the
 * handler selector at 0x80143FCD.
 * @source 0x80143FC8 @kind table
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];

/* Record allocator of the shared EXE image, outside this payload: it answers
 * the index of the next free 0x74-byte row of the front-end record table, or
 * 0xFF when every row is occupied. The original stores the answer straight into
 * the work byte and only then compares that byte, so this overlay sees the
 * byte-wide return type.
 */
u8 func_8019601C(void);

/* Shared update helper of the shared EXE image, outside this payload. This
 * body calls it on every entry and discards any result. */
void func_8014D978(void);

/* Cue dispatcher of the shared EXE image, outside this payload: it takes the
 * cue id in a whole word, the call passing the work halfword 0x2C biased by
 * 0x100 without narrowing or masking it. */
void func_8015DF18(u32 cue_id);

/* @source 0x801CE820
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044: it runs the shared update
 *           helper func_8014D978 and returns while the work halfword 0x58 does
 *           not read 0x0A. When it does, the body claims a front-end record
 *           through the allocator func_8019601C, stores the answer in the work
 *           byte 0x0B and, unless that byte reads 0xFF, seeds the 0x74-byte row
 *           at 0x80143FC8 it indexes: row byte 0 takes 1, the occupancy flag,
 *           and row byte 5 takes the handler selector 0x3A. Both stores rebuild
 *           the row offset from a fresh read of the work byte, so the whole
 *           multiply-by-0x74 chain runs twice. The body then hands the cue id
 *           0x100 plus the work halfword 0x2C to the cue dispatcher
 *           func_8015DF18, clears the work byte 0x07 and increments the work
 *           step counter byte 0x02. It takes no arguments and returns nothing.
 *           The 0x18-byte frame keeps $ra at 0x10, the 0xFF test carries the
 *           first shift of the row chain in its delay slot and the cue bias is
 *           materialised in the delay slot of its call. No jal in this payload
 *           names the body. Its only in-image reference is the word at
 *           0x801D079C, which holds 0x801CE820: that word is entry 4 of the
 *           twenty-four-word handler run based at 0x801D078C, which this
 *           payload's top dispatcher func_801CE404 indexes with work byte 2
 *           scaled by four, and it is at the same time entry 1 of the run based
 *           at 0x801D0798 that the byte-2 dispatcher 0x801CE610 indexes the
 *           same way. The two dispatchers therefore reach the body while work
 *           byte 2 reads 4 and 1 respectively, and the overlay loader that
 *           steps the outer run is its caller. The 268-byte span is
 *           byte-identical to the span at 0x801CE820 of plp578/00 and to the
 *           spans at 0x801CFD40 of plp025/00, 0x801CF548 of plp056/00,
 *           0x801CF018 of plp257/00 and 0x801CEA3C of plp457/00, compared as
 *           payload bytes rather than read from their metadata (md5
 *           adbbf3fc91d86961fc483553cf083486); every one of those six rows read
 *           exact when this seed was written, so the statement order, operand
 *           forms and declaration style here are that shared body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontQueuePlus(
    void) {
  func_8014D978();
  if (*(u16*)(D_1F800044 + 0x58) == 0x0A) {
    D_1F800044[0x0B] = func_8019601C();
    if (D_1F800044[0x0B] != 0xFFu) {
      D_80143FC8[(u32)D_1F800044[0x0B] * 0x74u] = 1;
      D_80143FCD[(u32)D_1F800044[0x0B] * 0x74u] = 0x3A;
    }
    func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
    D_1F800044[0x07] = 0;
    D_1F800044[0x02] += 1;
  }
}
