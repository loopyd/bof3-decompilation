#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with lb,
 * so it is declared signed here while every arm stores it with a plain byte
 * store. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared phase byte the waiting arms compare against 2 and latch back to 2. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared progress byte the last two arms gate on. */
extern u8 D_80146867; /* @source 0x80146867 @kind data */

/* Shared state-entry tuple the release arms reset. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Shared main-RAM flag bank the mode-10 arm sets bit 0x69 of. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

u32 func_80166140(u32 arg0);
void func_80150224(s32 action_id);
void func_8015B5F4(void* flag_bank, u8 bit_index);
void func_8015C058(void);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801BE1B0(u32 arg0);

/* @source 0x801F4B80
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay area-entry sequence, selecting the arm from the 14-slot
 *           jump table the payload keeps at 0x801F2C04. Modes 0, 1, 2, 10, 11,
 *           12 and 13 have bodies and the remaining slots return untouched.
 *           Mode 0 queries the shared selector predicate 0x80166140 with 6 and
 *           advances to 10 when its masked answer is nonzero, else to 1. Mode 1
 *           waits until the shared phase byte 0x80143BB0 differs from 2,
 *           requests master action 0x83 through 0x80150224, latches that phase
 *           byte to 2 and advances to 2. Mode 2 waits for the same phase
 *           difference and then resets the shared state-entry tuple: byte
 *           0x801448EB, the mode byte and byte 0x801448ED all clear before the
 *           shared front-end step 0x8015C058 runs. Mode 10 takes the same phase
 *           wait, requests action 0x82, latches the phase byte to 2, sets bit
 *           0x69 of the shared main-RAM flag bank 0x80144F28 through the shared
 *           setter 0x8015B5F4 and advances to 11. Mode 11 waits for the phase
 *           difference and runs the shared helper 0x801BE1B0 with 6 before
 *           advancing to 12. Mode 12 waits for the shared byte 0x80146867 to
 *           read 0x28 and then requests one of two front-selector contexts
 *           through 0x8019FA28 with action 0xBC: when the shared byte
 *           0x801448ED is set it passes 0x80000 and 0x220000 as kind 0x88,
 *           otherwise 0x330000 and 0x470000 as kind 0x87; it advances to 13.
 *           Mode 13 waits for 0x80146867 to read zero, runs 0x8015C058 and
 *           clears the same three state-entry bytes.
 *
 * Name evidence: 0x801448EC is the shared front-end mode byte the whole
 * `dispatchSharedModeByteTable2C04*` family steps, and 0x801F2C04 is the same
 * leading table-data base those overlays dispatch through, so this body takes
 * the family's own spelling with this overlay's suffix and the in-payload
 * address: dispatchSharedModeByteTable2C04World04Area18813_801F4B80. The three
 * in-repo members of that family (world02/area111@0x801F3EE8,
 * world03/area148@0x801F3C24, world04/area191@0x801F33C8) carry the identical
 * table base and mode byte, and this payload's 14 table words all point inside
 * this 0x208-byte span, so the range is one function and its table is data, not
 * a second entry point. No in-image `jal` reaches 0x801F4B80; the body is
 * reachable only from that table, so every role word stays structural.
 *
 * Shape: the dispatch prologue (`lb` of the signed mode byte, `sltiu` bound,
 * `sll`/`lui %hi(jtbl)`/`addu`/`lw`/`jr`) reproduces the family's reviewed
 * switch form, and the jump table is placed by the target's
 * `[[matching.section_placements]]` row for the `.rodata` section at
 * 0x801F2C04. The byte-wise pointer forms of modes 1 and 10 (address of the
 * phase byte held in $s0 across the action request) and the direct `lui`/`lbu`
 * reads of modes 2 and 11 follow the original access counts: the arms that
 * write the phase byte keep one address materialisation, the arms that only
 * read it reload it inline. Mode 12 tests the zero case first so the 0x330000
 * arm stays the fall-through of `bnez` with its `j` over the 0x80000 arm, which
 * is the branch the original bytes carry.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World04Area18813_801F4B80(void) {
  switch (D_801448EC) {
  case 0:
    if ((u8)func_80166140(6) != 0) {
      D_801448EC = 10;
      break;
    }
    D_801448EC = 1;
    break;
  case 1:
    {
      u8* phase;

      phase = &D_80143BB0;
      if (*phase == 2) {
        break;
      }
      func_80150224(0x83);
      *phase = 2;
      D_801448EC = 2;
    }
    break;
  case 2:
    if (D_80143BB0 == 2) {
      break;
    }
    D_801448EB = 0;
    D_801448EC = 0;
    D_801448ED = 0;
    func_8015C058();
    break;
  case 10:
    {
      u8* phase;

      phase = &D_80143BB0;
      if (*phase == 2) {
        break;
      }
      func_80150224(0x82);
      *phase = 2;
      func_8015B5F4(&D_80144F28, 0x69);
      D_801448EC = 11;
    }
    break;
  case 11:
    if (D_80143BB0 == 2) {
      break;
    }
    func_801BE1B0(6);
    D_801448EC = 12;
    break;
  case 12:
    if (D_80146867 != 0x28) {
      break;
    }
    if (D_801448ED == 0) {
      func_8019FA28(0xBC, 0x330000, 0x470000, 0x87);
    } else {
      func_8019FA28(0xBC, 0x80000, 0x220000, 0x88);
    }
    D_801448EC = 13;
    break;
  case 13:
    if (D_80146867 != 0) {
      break;
    }
    func_8015C058();
    D_801448EB = 0;
    D_801448EC = 0;
    D_801448ED = 0;
    break;
  }
}
