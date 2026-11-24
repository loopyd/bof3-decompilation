#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds it at
 * 0x1F800044 as g_battle_work, the name its lifted sources
 * clearWorkByte9SetWorkByteATo88AndIncrementWorkByte2BmagicMagic13703_801EFCC0
 * and advanceWorkByteBIncrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2
 * At192BmagicMagic13703_801EF174 already used. The original re-materialises the
 * cell before each access group because a store through the pointer it supplied
 * may alias the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared channel words of the main executable, outside this overlay image.
 * This target's generated undefined_syms_auto.txt already binds both
 * addresses, so no target-map row is added. */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */

/* Transform helper of the shared main executable: it consumes the motion offset
 * words this body stores at work 0xC and 0x10 and writes the transformed
 * offsets back to the same pair. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */

/* Distance helper of the shared main executable: it takes a signed position
 * pair and answers their distance as a halfword. This body adds 0x200 to the
 * answer with a plain 32-bit add and stores the result at halfword width. */
extern s32 func_8015477C(s32 x, s32 y); /* @source 0x8015477C */

/* Cue dispatcher of the shared main executable. */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801EF090
 * @behavior Seeds the scratchpad work object for the opening step of a
 *           battle-magic animation. The pointer cell at 0x1F800044 is read
 *           once, work word 0xC takes 0xFFFD8000 and work word 0x10 takes zero,
 *           and the transform helper 0x801DD350 runs on that pointer. The cell
 *           is re-read, work word 0x34 takes shared channel word 0x80149308
 *           plus work word 0xC and work word 0x38 takes shared channel word
 *           0x8014930C plus work word 0x10, publishing the transformed offset
 *           as an absolute position pair. The cell is read again for the
 *           halfword at 0x3E, which takes the shared distance helper 0x8015477C
 *           applied to the two channel words plus 0x200. Cue 0x100 is then
 *           dispatched through 0x8015DF18. Finally the body re-reads the cell
 *           for work byte 0xB, which takes 0x34, and work word 0x14, which takes
 *           0x2FFF, then clears work byte 9, sets work byte 0xA to 0x10 and
 *           increments work byte 2, the work-byte-2 step counter of the overlay
 *           dispatchers at 0x801EF018 and 0x801EFC58. It takes no arguments,
 *           returns nothing, keeps a 0x18-byte frame that holds only $ra, and
 *           writes no state outside the work object and the channel words it
 *           reads. Roles of work bytes 2, 9, 0xA, 0xB and words 0x14 and 0x3E
 *           stay address-anchored.
 *
 *           The statement order, operand forms and pointer re-read points are
 *           the exact emi/bmagic/magic132/03 lift at 0x801EF200, which seeds
 *           the same object through the same transform helper and publishes the
 *           same channel-word position pair; its work word 0x14 seed and byte
 *           0xB seed sit in the same order used here. This function's own
 *           constants, its halfword distance store and its cue argument come
 *           from this span's original bytes. No byte-identical twin exists: one
 *           payload comparison of the 0xE4 span against all 122 extracted
 *           emi/bmagic payloads reported no match.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateOffsetRearmFields(void) {
  u32 *work;
  u32 *position;
  u32 *rearm;

  work = (u32 *)g_battle_work;
  work[3] = 0xFFFD8000;
  work[4] = 0;
  func_801DD350((u8 *)work);
  position = (u32 *)g_battle_work;
  position[13] = D_80149308[0] + position[3];
  position[14] = D_8014930C[0] + position[4];
  *(s16 *)(g_battle_work + 0x3E) = func_8015477C(D_80149308[0], D_8014930C[0]) + 0x200;
  func_8015DF18(0x100);
  rearm = (u32 *)g_battle_work;
  ((u8 *)rearm)[0xB] = 0x34;
  rearm[5] = 0x2FFF;
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0x10;
  g_battle_work[2]++;
}
