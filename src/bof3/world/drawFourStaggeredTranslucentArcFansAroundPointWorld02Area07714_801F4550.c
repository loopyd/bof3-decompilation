#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident helpers this overlay calls, declared the way its siblings declare
 * them: func_8014E5A0 links the current primitive cursor into the cell list and
 * advances it by the passed size, func_801AFE18 fills the 0x20-byte projection
 * scratch handed as arg0, and func_801F462C draws one translucent triangle-fan
 * sector around the point handed as arg0, running from the signed start angle
 * handed as arg1 to the end of the 0x1000-unit turn in 0x80 steps and colouring
 * the fan hub with the three bytes of arg2 and its rim with those of arg3. */
void func_8014E5A0(u32 ot_index, u32 primitive_size);
void func_801AFE18(void* arg0);
void func_801F462C(const void* point, s16 start_angle, u8 base_color,
                   u8 tip_color);

/* @source 0x801F4550
 * @behavior Publishes the 0x380/0x100 texture page draw mode at the primitive
 *           cursor 0x8014598C through SetDrawMode with the dither field enabled
 *           and a null texture window, queues that 0xC-byte draw-mode packet
 *           under OT slot 1 with func_8014E5A0, and initialises the 0x20-byte
 *           projection scratch reserved at +0x18 of its 0x48-byte frame with
 *           func_801AFE18. It then runs the overlay's translucent fan drawer
 *           func_801F462C four times over the same point, once at each quarter
 *           of the handed signed angle: the fans start at angle/4, angle/2,
 *           3angle/4 and angle and each runs on to the end of the 0x1000-unit
 *           turn, so the four staggered sectors cover successively shorter
 *           arcs. Every fan is drawn with 0x20 in all three hub colour channels
 *           and zero in the rim ones. Takes the point record pointer and the
 *           signed 16-bit angle; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourStaggeredTranslucentArcFansAroundPointWorld02Area07714_801F4550(
    const void* point, s16 angle) {
  u8 scratch[0x20];

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  func_8014E5A0(1, 0xC);

  func_801AFE18(scratch);

  func_801F462C(point, angle >> 2, 0x20, 0);
  func_801F462C(point, (angle * 2) >> 2, 0x20, 0);
  func_801F462C(point, (angle * 3) >> 2, 0x20, 0);
  func_801F462C(point, angle, 0x20, 0);
}
