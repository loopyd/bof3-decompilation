#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. This target's map binds it at
 * 0x1F800044 as g_battle_work, the name its lifted sources
 * advanceWorkByteBIncrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2At192
 * BmagicMagic06703_801EF44C and
 * advanceWorkByteBDecrementByteAOnGlobalBit0AndByte9ByTwoThenAdvanceByte2At96
 * BmagicMagic06703_801EF4F4 already use. The original re-materialises the cell
 * before each access group because a store through the pointer it supplied may
 * alias the cell, so the body reads the named cell each time instead of holding
 * one copy. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared fixed-point pair in main RAM, outside this overlay image, bound by
 * this target's generated undefined_syms_auto.txt rows. This body adds the two
 * words to work words 0xC and 0x10 and stores the sums into work words 0x34 and
 * 0x38, so the pair is read here at scalar width, once per sum, and its role
 * stays address-anchored. */
extern u32 D_80149308; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C; /* @source 0x8014930C @kind unknown */

/* Shared main-executable helpers, all outside this overlay image and bound by
 * this target's generated undefined_funcs_auto.txt rows. func_800AFD9C is
 * invoked first with no argument set up and its result discarded, func_801DD350
 * takes the scratchpad work pointer whose step words this body has just seeded,
 * and func_8015DF18 takes the cue 0x100 this body passes in the first argument
 * register. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern void func_8015DF18(u32 cue); /* @source 0x8015DF18 */

/* @source 0x801EF38C
 * @behavior Seeds the scratchpad work object for one stage of a battle-magic
 *           animation and re-arms its countdown fields. It first invokes the
 *           shared main-executable helper func_800AFD9C with no arguments, then
 *           reads the pointer cell at 0x1F800044 once: work word 0xC takes
 *           0xFFFC0000 and work word 0x10 takes zero, the zero store filling
 *           the call delay slot of func_801DD350, which runs on that pointer.
 *           The cell is re-read after the call, and with that one pointer the
 *           body publishes the absolute position pair: work word 0x34 takes the
 *           shared word 0x80149308 plus work word 0xC and work word 0x38 takes
 *           the shared word 0x8014930C plus work word 0x10, the second sum
 *           filling the call delay slot of func_8015DF18, which is invoked with
 *           cue 0x100. Finally the body re-reads the cell for each access group
 *           of the rearm: work byte 0xB takes 0x34 and work word 0x14 takes
 *           0x2FFF through the first re-read, work byte 9 is cleared through the
 *           second, and the third serves work byte 0xA, which takes 0x10, and
 *           work byte 2, which is incremented. It takes no arguments, returns
 *           nothing, keeps a 0x18-byte frame that holds only $ra, and writes no
 *           state outside the work object and the shared pair it reads. No jal
 *           in the payload reaches it and no pointer word names it: its address
 *           is entry 0 of the four-word code pointer run at 0x801F04C8 that the
 *           dispatcher at 0x801EF314 indexes by work byte 2, so the parameter
 *           roles stay structural.
 *
 *           The statement order and pointer re-read points are those of the
 *           exact emi/bmagic/magic137/03 lift at 0x801EF090, whose 0xC0-byte
 *           span differs from this one in 83 bytes, all of them inside the
 *           leading 0x64 bytes: that body stores the step word 0xFFFD8000,
 *           indexes the shared pair through a register-held array base and
 *           carries an extra distance halfword at work 0x3E that this span does
 *           not have, while this span opens with the func_800AFD9C call and
 *           reads the shared pair at scalar width. Its rearm tail, from work
 *           byte 0xB to the increment of work byte 2, is byte-identical to that
 *           sibling's. No byte-identical twin exists: a payload comparison of
 *           this 0xC0 span against every extracted emi/bmagic payload reported
 *           no match. Roles of work bytes 2, 9, 0xA and 0xB, of work words 0xC,
 *           0x10, 0x14, 0x34 and 0x38 and of both shared helpers stay
 *           address-anchored.
 *
 *           Representation: the two position sums are written through the work
 *           pointer's byte view, one pointer read for both stores. Measured
 *           contrast on this span: the element form
 *           position[13] = D_80149308 + position[3] with a u32 pointer local
 *           scored 39/48 instructions and 188 bytes, leaving first difference at
 *           +0x28 where the compiler hoisted the second pair load, the second
 *           step load and the cue argument above the first store, and dropping
 *           the load-delay nop after work word 0xC's read. The byte-offset form
 *           reproduced the serialized expansion and the nop.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedMotionOffsetPositionPairCue100AndRearmFieldsThenAdvanceWorkByte2BmagicMagic06703_801EF38C(void) {
  u32 *work;
  u8 *position;
  u32 *rearm;

  func_800AFD9C();
  work = (u32 *)g_battle_work;
  work[3] = 0xFFFC0000;
  work[4] = 0;
  func_801DD350((u8 *)work);
  position = g_battle_work;
  *(u32 *)(position + 0x34) = D_80149308 + *(u32 *)(position + 0xC);
  *(u32 *)(position + 0x38) = D_8014930C + *(u32 *)(position + 0x10);
  func_8015DF18(0x100);
  rearm = (u32 *)g_battle_work;
  ((u8 *)rearm)[0xB] = 0x34;
  rearm[5] = 0x2FFF;
  g_battle_work[9] = 0;
  g_battle_work[0xA] = 0x10;
  g_battle_work[2]++;
}
