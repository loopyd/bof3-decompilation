#include "bof3/bof3.h"

/* Shared scratchpad work record pointer cell at 0x1F800044. The original reads
 * it once at entry for the +0x09 countdown byte and then through its own load
 * for every later access, because each preceding store goes through the loaded
 * pointer and may alias the cell itself.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared channel halfword at 0x8014932C. Entry 0 of this overlay's byte-1
 * handler run seeds it with 0x1C80, this body subtracts 0x2A from it per frame
 * and stores 0x400 when the countdown byte runs out. Store width comes from the
 * original's single lhu/sh pair.
 * @source 0x8014932C @kind data
 */
extern u16 D_8014932C;

/* Shared mode byte at 0x80149333. This body publishes 2, the value entry 0 of
 * the run also writes, and the scenario overlays write the same cell. Store
 * width comes from the original's single sb.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Local drawing helper at 0x801F2DE0. It builds a tile from GetTPage,
 * SetDrawMode, SetTile, SetSemiTrans and SetShadeTex and takes the three level
 * bytes 0x5D, 0x5E and 0x5F as the tile's colour channels, so this step calls it
 * after updating them. */
void func_801F2DE0(void);

/* @source 0x801F2CD4
 * @behavior Entry 2 of this overlay's byte-1 handler run based at 0x801F37D0,
 * the four-word pointer table whose words are 0x801F2C48, 0x801F2CB4,
 * 0x801F2CD4 and 0x801F2DAC in that order and which the exact dispatcher
 * callStateTable37World03 indexes with the
 * unsigned work byte at +0x01 of the record published at the scratchpad pointer
 * cell 0x1F800044, so this body is the step that dispatcher takes while that
 * byte reads 2. It decrements the record countdown byte +0x09; when that byte
 * reaches 0 it stores 0x400 into the shared channel halfword 0x8014932C and
 * stores 3 into the record dispatch byte +0x01, which hands the record to entry
 * 3 (0x801F2DAC) on the next frame. While the countdown still runs it subtracts
 * 0x2A from the channel halfword and, when the signed level byte +0x5D is
 * nonzero, decrements that byte and the two following level bytes +0x5E and
 * +0x5F, each of the latter two through its own fresh load of the pointer cell.
 * Whether or not the countdown expired it then stores 2 into the shared mode
 * byte 0x80149333 and calls the local drawing helper func_801F2DE0, which
 * renders a tile from the three level bytes. Takes no arguments and returns
 * nothing; the 0x20-byte frame holds only $ra at +0x18 and the 0x18 bytes below
 * it are never accessed.
 *
 * Name evidence: the record offset +0x01 this body writes is the selector of
 * the byte-1 run at 0x801F37D0 (entry 0 of the run
 * stateChannelFlagsTriple
 * stores 1 there to hand the record to entry 1, and entry 3 is 0x801F2DAC), and
 * the countdown byte +0x09 is the one that entry 0 arms with 0x96, so the two
 * arms of this step are the running countdown and the handover to entry 3. The
 * channel halfword 0x8014932C and the mode byte 0x80149333 are the shared cells
 * the same run writes.
 *
 * Shape evidence: no payload under out/binaries carries this 216-byte span, so
 * there is no byte-identical sibling to clone from and the body is written from
 * these original bytes. The run arm is the taken-fallthrough arm (the original
 * tests the decremented countdown with a zero branch to the handover block that
 * sits after it), the countdown result is re-read as a u8 before the test, the
 * level byte +0x5D is read signed while the two bytes after it are read
 * unsigned, and every access after the entry load re-reads the pointer cell
 * because the store before it goes through the loaded pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepLowerLevelTriple(
    void) {
  u8* work;

  work = D_1F800044;
  if (--work[9] != 0) {
    s8* level;

    D_8014932C -= 0x2A;
    level = (s8*)(D_1F800044 + 0x5D);
    if (*level != 0) {
      *level = *level - 1;
      D_1F800044[0x5E] -= 1;
      D_1F800044[0x5F] -= 1;
    }
  } else {
    D_8014932C = 0x400;
    D_1F800044[1] = 3;
  }
  D_80149333 = 2;
  func_801F2DE0();
}
