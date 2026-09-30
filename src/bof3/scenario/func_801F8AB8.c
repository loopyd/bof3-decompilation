#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8AB8
 * @behavior Advances the scratchpad work object published at 0x1F800044 by one
 * trail step: adds 0x20 to the object's depth halfword at offset 0x3E,
 * projects the object's cursor words at offsets 0x34/0x38/0x3C through the
 * shared projector func_801AFF04 into the scratchpad screen pair at
 * 0x1F800034/0x1F800036, copies that pair back into the object's halfwords at
 * offsets 0x2E and 0x30 and, when the object's signed tint byte at offset 0x5D
 * is nonzero, fades the three tint bytes at offsets 0x5D/0x5F/0x5E by 4 each,
 * increments the object's transition byte at offset 0x06 once the low three
 * bits of its byte at offset 0x09 are clear and runs the following handler
 * func_801F8BCC, otherwise resets the shared work flags through func_80196070.
 * Takes no arguments and returns nothing; the address is entry 25 (word at
 * 0x801FC9E4) of the per-frame handler table at 0x801FC980 indexed by byte 0x01
 * of that work object.
 * @status exact
 * @match 100.00
 * @residual none
 * Live audit: 69/69 instructions, 276 -> 276 bytes. The four leading accesses
 * spell the published pointer cell D_1F800044 directly, and the halfword store
 * at offset 0x3E does not invalidate that cell, so one `lui/lw` pair is emitted
 * and reused for all four accesses exactly as the original does; binding the
 * same block to a local (`u8* work = D_1F800044;`) gave the pointer $a0 and the
 * cursor-x load $v1 instead (61/69, first=+0x0000). The byte stores in the fade
 * arm do invalidate the cell, so those keep a fresh load per statement.
 */
void func_801F8AB8(void) {
  VECTOR point;
  long*  sxy;
  s8     fade;

  sxy = (long*)&D_1F800034;
  *(u16*)(D_1F800044 + 0x3E) += 0x20;
  point.vx = *(s32*)(D_1F800044 + 0x34);
  point.vy = *(s32*)(D_1F800044 + 0x38);
  point.vz = *(s32*)(D_1F800044 + 0x3C);
  func_801AFF04(&point, sxy);
  *(u16*)(D_1F800044 + 0x2E) = *(u16*)sxy;
  *(u16*)(D_1F800044 + 0x30) = D_1F800036;
  fade = *(s8*)(D_1F800044 + 0x5D);
  if (fade != 0) {
    *(s8*)(D_1F800044 + 0x5D) = fade - 4;
    *(u8*)(D_1F800044 + 0x5F) -= 4;
    *(u8*)(D_1F800044 + 0x5E) -= 4;
    if ((D_1F800044[9] & 7) == 0) {
      D_1F800044[6]++;
    }
    func_801F8BCC();
  } else {
    func_80196070();
  }
}
