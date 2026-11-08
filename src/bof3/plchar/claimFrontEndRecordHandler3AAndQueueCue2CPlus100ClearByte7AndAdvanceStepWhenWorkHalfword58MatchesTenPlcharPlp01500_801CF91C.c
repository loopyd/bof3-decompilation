#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell before each of the six accesses of this body,
 * because a store through the pointer may alias the cell itself, so the source
 * keeps the named-cell form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Front-end record table of the shared main-executable data segment: 0x74-byte
 * rows based at 0x80143FC8. This body addresses two fields through their own
 * absolute symbols, because the original re-materialises each base and adds the
 * scaled row offset again for the second store instead of keeping one base.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* Row byte 5, the entry handler selector, seeded here with 0x3A.
 * @source 0x80143FCD @kind unknown
 */
extern u8 D_80143FCD[];

/* External no-argument update of the shared main executable, called before the
 * work halfword test of this body. include/bof3/ui/game00_internal.h declares
 * the same no-argument shape and an unsigned byte answer whose meaning is
 * unknown; this body discards that answer.
 * @source 0x8014D978
 */
extern u8 func_8014D978(void);

/* Front-end record allocator of the shared main-executable data segment: it
 * answers the index of the next free 0x74-byte row at 0x80143FC8, or 0xFF when
 * every row is occupied. The sibling lifts of this overlay declare the same
 * shape.
 * @source 0x8019601C
 */
extern u8 func_8019601C(void);

/* Cue dispatcher of the shared main executable: the sibling lifts of the
 * scenario and world payloads pass sound cue ids to 0x8015DF18. This call site
 * passes the inherited work halfword 0x2C with a plain 32-bit add of 0x100 and
 * the original masks nothing afterwards, so the width here is the full word the
 * caller forms; the boss lift at 0x800C1A78 declares the same word width, and
 * the halfword declaration in include/bof3/scenario/sce10eff_internal.h belongs
 * to call sites that pass constants.
 * @source 0x8015DF18
 */
extern void func_8015DF18(u32 cue);

/* @source 0x801CF91C
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044. It runs the shared external
 *           update func_8014D978 first and only then tests the work halfword
 *           0x58; when that halfword reads 10 (0x0A) it claims a front-end
 *           record by asking the shared allocator func_8019601C for a free
 *           0x74-byte row index, storing that answer in work byte 0x0B and, if
 *           the byte is not 0xFF, seeding the row at 0x80143FC8: occupancy byte
 *           0 takes 1 and handler selector byte 5 takes 0x3A. Whether or not a
 *           row was claimed it then queues the cue whose id is the work
 *           halfword 0x2C plus 0x100 through the shared cue dispatcher
 *           func_8015DF18, clears work byte 0x07 and increments the step
 *           counter byte 0x02, which advances the sequence past this step. A
 *           work halfword 0x58 that reads anything else leaves the object
 *           untouched and returns right after the external update. It takes no
 *           arguments and returns nothing.
 *
 * Shape notes from the original bytes: the 0x10C-byte body spends all six of
 * its pointer-cell loads on the accesses above, one per access, and its 0x18
 * frame keeps only $ra at 0x10 because no local outlives a call. The claimed
 * row index is read back from work byte 0x0B for each store, so the original
 * reloads the cell and the byte between the two field seeds, and the scaled
 * offset kept in $v0 at each of them is the compiler's multiply-by-0x74 chain
 * (sll 3, subu, sll 2, addu, sll 2) whose first shift sits in the delay slot of
 * the 0xFF test. The cue id is formed with a plain 32-bit add of 0x100 to the
 * zero-extended halfword, with no mask afterwards, which the compiler places in
 * the call's delay slot.
 * The body's address appears once in this payload, as word 0x801CFBC0, entry 15
 * of the sixteen-word pointer run that starts at 0x801CFB84, so it is the last
 * entry of that run and is reached from outside the payload; its own prologue
 * and return path alone place the boundary. The same 0x10C bytes are present in
 * the plp025, plp056, plp257, plp457, plp567 and plp578 overlays; none of them
 * held a lifted source when this seed was authored, so there was no sibling
 * body to clone and the seed comes from the original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFrontEndRecordHandler3AAndQueueCue2CPlus100ClearByte7AndAdvanceStepWhenWorkHalfword58MatchesTenPlcharPlp01500_801CF91C(
    void) {
  u32 row;
  u8 index;

  func_8014D978();
  if (*(u16*)(D_1F800044 + 0x58) != 10) {
    return;
  }
  D_1F800044[0xB] = func_8019601C();
  index = D_1F800044[0xB];
  if (index != 0xFF) {
    row = (u32)index * 0x74;
    D_80143FC8[row] = 1;
    row = (u32)D_1F800044[0xB] * 0x74;
    D_80143FCD[row] = 0x3A;
  }
  func_8015DF18(*(u16*)(D_1F800044 + 0x2C) + 0x100);
  D_1F800044[7] = 0;
  D_1F800044[2] = D_1F800044[2] + 1;
}
