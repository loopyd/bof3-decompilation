#include "bof3/bof3.h"

/* Scratchpad pointer cell of the work record this arm seeds. The original
 * re-reads the cell through its own address load for each of the six store
 * groups, so every group re-reads the symbol instead of reusing one pointer.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;
/* Pointer cell of the shared record whose halfword at 0x0A this arm consumes.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* @source 0x801F31E4
 * @behavior Arming entry of the four-word work-byte-4 handler table at
 *           0x801F41DC, the entry the neighbouring
 *           callWorkByte4Table41 dispatcher
 *           selects with byte 0x04 value 0, which is the value the raise step
 *           at 0x801F3278 stores there once its countdown reaches zero. It
 *           re-reads the scratchpad pointer cell at 0x1F800044 for each of six
 *           store groups: it stores 2 into the record byte at 0x48; 0x10000
 *           into each of the words at 0x40 and 0x44, the pair the raise and
 *           fall steps then move by 0x2000 per frame; 0xFF into the countdown
 *           byte at 0x0A, the count that raise step decrements; 1 into the
 *           word at 0x70; 0x50, 0x3C and 0 into the colour-channel bytes at
 *           0x5D, 0x5E and 0x5F that
 *           renderFullscreenColorTile shades
 *           with; and 1 into the record byte at 0x04, the selector that hands
 *           the record to the raise step. It then subtracts two from the
 *           halfword at 0x0A of the shared record published through the
 *           pointer cell at 0x80148210, the same cell the raise step consumes
 *           while a countdown remains. Takes no arguments, calls nothing and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armRiseTintRise(
    void) {
  u8* work;
  u8* step;
  u8* tint;
  u8* green;
  u8* blue;
  u8* select;

  work = D_1F800044;
  work[0x48] = 2;
  step = D_1F800044;
  *(u32*)(step + 0x40) = 0x10000;
  *(u32*)(step + 0x44) = 0x10000;
  step[0x0a] = 0xFF;
  tint = D_1F800044;
  *(u32*)(tint + 0x70) = 1;
  tint[0x5d] = 0x50;
  green = D_1F800044;
  green[0x5e] = 0x3C;
  blue = D_1F800044;
  blue[0x5f] = 0;
  select = D_1F800044;
  select[0x04] = 1;
  D_80148210[5] -= 2;
}
