#include "bof3/bof3.h"

/* The overlay's work record is published through the scratchpad pointer cell at
 * 0x1F800044, the same cell the sibling steps of this handler table use. The
 * constant-address accessor is not used here: this function re-reads the cell
 * once per group, so the named symbol lets the assembler re-materialise
 * `lui $vN,0x1F80` plus the folded `lw $vN,0x44($vN)` at each of them. The cell
 * is a plain pointer: the stores through the loaded record already separate the
 * reads, so no volatility is needed. */
extern u8* workRecord44World02Area10814; /* @source 0x1F800044 @kind data */

/* The shared projector 0x801AFF04 writes the projected screen pair into the
 * scratchpad halfwords at 0x1F800034 and 0x1F800036, which this step hands to
 * it as its destination and reads back into the record's own halfwords at
 * +0x2E and +0x30. Both halves are bound as their own cells so the pair base
 * keeps a single address materialisation across the call. */
extern u16 workScreenX34World02Area10814; /* @source 0x1F800034 @kind data */
extern u16 workScreenY36World02Area10814; /* @source 0x1F800036 @kind data */

/* Shared overlay helpers. 0x801AFF04 projects a three-word point into a screen
 * pair, and 0x801F3D90 draws the semi-transparent circle fan of radius arg2
 * around (arg0, arg1) with one colour for the inner and one for the outer
 * edge. Both are still unlifted assembly inside this target, so only their
 * prototypes are declared. */
void func_801AFF04(const void* arg0, void* arg1);
void func_801F3D90(s16 arg0, s16 arg1, s16 arg2, u8 arg3, u8 arg4);

/* @source 0x801F384C
 * @behavior Overlay step handler, entry 2 of the local state-byte-1 handler
 *           table at 0x801F56FC that the 0x801F36A4 dispatcher indexes with
 *           byte +0x01 of the work record published at scratchpad pointer cell
 *           0x1F800044. It projects the record's cursor words at +0x34, +0x38
 *           and +0x3C through the shared projector 0x801AFF04 into the
 *           scratchpad screen pair at 0x1F800034/0x1F800036, copies that pair
 *           into the record's halfwords +0x2E (projected x) and +0x30
 *           (projected y) and paints a white circle of radius halfword +0x18
 *           around it through the circle helper 0x801F3D90, whose inner and
 *           outer colours are both the constant 0xFF. It then decrements the
 *           record's frame countdown byte +0x09 and, when that byte reaches
 *           zero, re-reads the cell to seed the tint triplet at +0x5F, +0x5E
 *           and +0x5D with the white level 0x60 and to advance the record's
 *           state byte +0x01 by one, which hands the next frame to entry 3 at
 *           0x801F3920. That sibling shrinks the same radius halfword by 0x0A
 *           per frame while the previous entry 0x801F376C grows it by 0x05 and
 *           seeds this countdown byte with 0x0F once the radius passes 0x190,
 *           and the following fade step 0x801F3A6C subtracts 6 per frame from
 *           the triplet seeded here, so the three addresses are one flash
 *           sequence. Takes no arguments, returns nothing and keeps the
 *           0x20-byte frame that holds $s0 and $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWhiteCircleSeedTintTripletAndAdvanceWorkByte1WhenCountdownExpiresWorld02Area10814_801F384C(void) {
  u16* screen;
  u16 screenY;
  u8* record;
  u8* tint;

  screen = &workScreenX34World02Area10814;
  func_801AFF04(workRecord44World02Area10814 + 0x34, screen);
  record = workRecord44World02Area10814;
  *(u16*)(record + 0x2E) = *screen;
  screenY = workScreenY36World02Area10814;
  *(u16*)(record + 0x30) = screenY;
  func_801F3D90(*(s16*)(record + 0x2E), (s16)screenY, *(s16*)(record + 0x18),
                0xFF, 0xFF);
  if (--workRecord44World02Area10814[9] == 0) {
    tint = workRecord44World02Area10814;
    tint[0x5F] = 0x60;
    tint[0x5E] = 0x60;
    tint[0x5D] = 0x60;
    workRecord44World02Area10814[1]++;
  }
}
