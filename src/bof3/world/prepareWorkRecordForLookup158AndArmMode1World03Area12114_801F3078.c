#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;

void func_8014DD3C(u16 entry);

/* @source 0x801F3078
 * @behavior Mode-0 handler of the work-record mode table at 0x801F67C4, which
 * the overlay's update at 0x801F2F04 indexes with the work-record byte at 0x01.
 * It stores 0x80 in work byte 0x24, applies main lookup entry 0x158 through the
 * resident func_8014DD3C, stores 5 in work byte 0x29, clears work bytes 0x2A and
 * 0x5D..0x5F, and finally arms the mode byte at 0x01 with 1 so the next dispatch
 * runs the mode-1 handler. The scratchpad pointer cell 0x1F800044 is re-read
 * before every store, matching the original's per-store reloads. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void prepareWorkRecordForLookup158AndArmMode1World03Area12114_801F3078(void) {
  area12114WorkCursor_1F800044[0x24] = 0x80;
  func_8014DD3C(0x158);
  area12114WorkCursor_1F800044[0x29] = 5;
  area12114WorkCursor_1F800044[0x2A] = 0;
  area12114WorkCursor_1F800044[0x5D] = 0;
  area12114WorkCursor_1F800044[0x5E] = 0;
  area12114WorkCursor_1F800044[0x5F] = 0;
  area12114WorkCursor_1F800044[0x01] = 1;
}
