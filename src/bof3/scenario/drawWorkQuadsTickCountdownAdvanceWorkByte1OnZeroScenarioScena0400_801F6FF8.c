#include "bof3/bof3.h"

void func_801F7084(u8* record);

/* @source 0x1F800044 @kind unknown */
extern u8* D_1F800044;

/* @source 0x801F6FF8
 * @behavior Entry 4 of this overlay's nine-word work-byte-1 handler run at
 *           0x801FA180 ({0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C,
 *           0x801F6FF8, 0x801F7064, 0x801F77EC, 0x801F78F4, 0x801F7A64}), so
 *           the dispatcher dispatchWorkByte1HandlerScenarioScena0400_801F6D00
 *           reaches it only through its indirect call; entry 0
 *           (seedWorkFieldsAndSegmentKeyTableSetByte9AndAdvanceWorkByte1ScenarioScena0400_801F6D44)
 *           arms work byte 9 with 8 on the way into this run, and each frame
 *           that holds byte 1 at 4 runs one step of that countdown.
 *           It loads the work-object pointer from the scratchpad cell
 *           0x1F800044, passes the record at its +0x0C to the quad-primitive
 *           builder at 0x801F7084 (that body sets a tpage and a draw mode,
 *           writes eight vertex records from the record's position words at
 *           +0x00/+0x04/+0x08/+0x10/+0x14 and its extent words at
 *           +0x18/+0x1C/+0x20/+0x28/+0x2C, and emits five vertex-indexed
 *           quads through 0x801F732C), then re-reads the cell and decrements
 *           the work object's unsigned countdown byte 9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb) and tests the stored value
 *           against zero (andi 0xFF / bnez). Only when the countdown reaches
 *           zero does it re-read the cell a third time and increment work byte
 *           1 by one, so the next frame dispatches entry 5 at 0x801F7064. The
 *           cell is re-read per access because the byte stores through the
 *           loaded pointer may touch it, so the original re-materialises
 *           `lui / lw 0x44` three times instead of holding the cell address.
 *           It takes no arguments, consumes no result, and its 0x18-byte frame
 *           only keeps $ra across the call, which is why the argument setup for
 *           that call precedes the frame allocation in the original.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkQuadsTickCountdownAdvanceWorkByte1OnZeroScenarioScena0400_801F6FF8(
    void) {
  func_801F7084(D_1F800044 + 0xC);
  if (--D_1F800044[9] == 0) {
    D_1F800044[1]++;
  }
}
