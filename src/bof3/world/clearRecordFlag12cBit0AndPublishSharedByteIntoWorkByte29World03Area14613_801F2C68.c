#include "bof3/bof3.h"

/* Pointer cell of the shared main-executable record whose bits this body clears.
 * Its role beyond byte 0x12C is not established, so the cell stays
 * address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared main-executable byte this body publishes into the work record. Its role
 * is not established, so it stays address-anchored.
 * @source 0x8014933E @kind unknown
 */
extern u8 D_8014933E;

/* @source 0x801F2C68
 * @behavior Entry 2 of the three-entry handler run at 0x801F3AB0, which the
 *           overlay names through the pointer word at 0x801F3AF8: clears bit 0
 *           of the flags byte at 0x12C of the record published at 0x80146250,
 *           then stores the shared main-executable byte 0x8014933E into byte
 *           0x29 of the work record reached through the scratchpad pointer cell
 *           0x1F800044. It takes no arguments, returns nothing, calls nothing
 *           and reserves no frame. The original reloads the scratchpad cell
 *           after the record store, fills that load delay slot with the 0x8014933E
 *           byte load and leaves that byte store in the jr $ra delay slot. Its
 *           only reference in the payload is the table word at 0x801F3AB8, so the
 *           handler is reached from outside the payload and its argument roles
 *           stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearRecordFlag12cBit0AndPublishSharedByteIntoWorkByte29World03Area14613_801F2C68(void) {
  u8* record;

  record = D_80146250;
  record[0x12C] &= 0xFE;
  SPAD_PTR_SLOT(u8, 0x44u)[0x29] = D_8014933E;
}
