#include "bof3/bof3.h"

/* Scratchpad pointer cell that publishes the overlay work record. Declared
 * plain, not volatile: the second read of this body follows a byte store, and a
 * byte store may alias the cell itself, so the compiler re-materialises the
 * cell's own lui/lw pair instead of holding the pointer. The same binding and
 * the same reload point measured exact in this target's step at 0x801F3FC0. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void stateWorkTableStride28(void);
void clearWorkTableAt800(void);

/* 0x801F3110 - this overlay's random position seeder for the 0x28-byte-stride
 * table at 0x800E4800. It walks the entry index from the first argument while
 * that index stays below the second, masks both to a halfword, and for each
 * entry takes the three position words from the two vector pointers the last
 * two arguments name: it calls rand() per component and interpolates between the
 * source word and the destination word, also setting the entry lead byte and the
 * three bytes before the record base from further rand() results. The span is
 * still an asm row in this target, so only the argument forms are declared
 * here. */
void setupWorkPositionSeeds(u16 start, u16 end, s32* from, s32* to); /* @source 0x801F3110 */

/* 0x801F34A0 - the same random position seeding for the 0x24-byte-stride table
 * at 0x800E5C00, with the same four arguments and the same rand() per
 * component. The span is still an asm row in this target. */
void setupEffectPositionSeeds(u16 start, u16 end, s32* from, s32* to); /* @source 0x801F34A0 */

/* 0x8015DF18 - main-executable cue request. The reviewed lifts of that function
 * in this repository pass it one cue number as its only argument. */
void func_8015DF18(u32 cue);

/* @source 0x801F2C48
 * @behavior Overlay init handler of this payload: entry 0 of the state-byte-1
 *   handler run at 0x801F431C, which the dispatcher at 0x801F2C04 reaches
 *   through the work record byte at +0x01 of the scratchpad pointer cell
 *   0x1F800044. It empties the 0x28-byte-stride table at 0x800E4800, empties
 *   the 0x24-byte-stride table at 0x800E5C00, and then seeds both tables with
 *   random position words in three entry bands: entries 0 to 0x1F interpolate
 *   between the X words 0x20000 and 0x50000, entries 0x20 to 0x5F between
 *   0x50000 and 0xA0000, and entries 0x60 to 0x7F between 0xA0000 and 0xD0000,
 *   with the source word pair 0x770000 and 0xFF000000 and the destination word
 *   pair 0x770000 and 0x2000000 held the same in all three bands. It then
 *   requests cue 0x207 through func_8015DF18, stores 0 in the work record byte
 *   at +0x02 so the second handler run at 0x801F4368 restarts at its entry 0,
 *   re-reads the pointer cell and stores the byte at +0x01 incremented by 1, so
 *   its own run advances to entry 1 at 0x801F2DA4. It takes no arguments and
 *   returns nothing; the 0x48-byte frame holds the two three-word vectors at
 *   0x10 and 0x20 and keeps the return address with five callee-saved
 *   registers, four of them holding the words 0x770000, 0xFF000000, 0x2000000
 *   and the band start word, which the compiler keeps live across all six seed
 *   calls because every band stores all six words again.
 *
 * Name evidence: the only word of this payload that names 0x801F2C48 is the
 *   first of the handler run at 0x801F431C (payload offset 0x171C), and the
 *   dispatcher at 0x801F2C04 indexes that run with the work record byte at
 *   +0x01 (lbu, sll 2, addu on the run base, lw, jalr). This body advances
 *   exactly that byte, from the 0 the run is entered with to 1, and clears the
 *   byte at +0x02 that the second dispatcher at 0x801F3628 reads, so it is the
 *   init step of both runs. The tables it clears and seeds are the ones this
 *   target's own reviewed helpers empty: 0x800E4800 through
 *   stateWorkTableStride28 and 0x800E5C00
 *   through clearWorkTableAt800, which is why the
 *   name keeps that module vocabulary. Cue 0x207 is the immediate this body
 *   passes to func_8015DF18; its game meaning is not established by any reviewed
 *   record of this target, so the name keeps the number.
 *
 * Shape evidence: a payload scan over all extracted binaries under out/binaries
 *   found this 348-byte body once, at file offset 0x48 of this target, so no
 *   byte-identical sibling exists and the clone lever is declined. A second scan
 *   that compares opcode fields alone, and a third that compares opcode and
 *   register fields while ignoring immediates, each match this span only here.
 *   The body was written from the original bytes of this span. Probes kept both
 *   losing spellings: assigning the two word pairs once before the first band
 *   measured 45/87 with a 0x40-byte frame, because the compiler then
 *   re-materialised each constant instead of holding it, and assigning all six
 *   words in every band measured 84/87 with the frame at 0x48; the first store
 *   through one pointer local and the second through its own local then measured
 *   87/87 exact, which is the register choice this body needs for the first
 *   dereference of the cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initWorkTablesSeed(
    void) {
  s32 from[3];
  s32 to[3];
  u8* secondRun;
  u8* firstRun;

  stateWorkTableStride28();
  clearWorkTableAt800();

  from[0] = 0x20000;
  from[1] = 0x770000;
  from[2] = 0xff000000;
  to[0] = 0x50000;
  to[1] = 0x770000;
  to[2] = 0x2000000;
  setupWorkPositionSeeds(0, 0x20, from, to);
  setupEffectPositionSeeds(0, 0x20, from, to);

  from[0] = 0x50000;
  from[1] = 0x770000;
  from[2] = 0xff000000;
  to[0] = 0xa0000;
  to[1] = 0x770000;
  to[2] = 0x2000000;
  setupWorkPositionSeeds(0x20, 0x60, from, to);
  setupEffectPositionSeeds(0x20, 0x60, from, to);

  from[0] = 0xa0000;
  from[1] = 0x770000;
  from[2] = 0xff000000;
  to[0] = 0xd0000;
  to[1] = 0x770000;
  to[2] = 0x2000000;
  setupWorkPositionSeeds(0x60, 0x80, from, to);
  setupEffectPositionSeeds(0x60, 0x80, from, to);

  func_8015DF18(0x207);

  secondRun = D_1F800044;
  secondRun[2] = 0;
  firstRun = D_1F800044;
  firstRun[1] = firstRun[1] + 1;
}
