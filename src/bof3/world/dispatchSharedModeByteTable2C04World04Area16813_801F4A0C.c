#include "bof3/bof3.h"

/* Shared front-end mode byte this overlay steps. The dispatch reads it with
 * `lb`, so it is declared signed here while every arm stores it with a plain
 * byte store. This target's own setStatusFFFFAndStateEntry* neighbours, which
 * request entry 0x3B and entry 0x0A, call 0x801448EB the state-entry byte and
 * 0x801448EC its progress byte; the family name below keeps the mode wording
 * its four reviewed siblings in world02/area111, world03/area148,
 * world04/area188 and world04/area191 use for the same byte and table base. */
extern s8 D_801448EC; /* @source 0x801448EC @kind data */

/* Shared phase byte the first arm waits on. 0x8019FA28 latches it to 5 and
 * the sibling arms of this family latch it to 2. */
extern u8 D_80143BB0; /* @source 0x80143BB0 @kind data */

/* Shared state-entry byte the reset tail clears beside the mode byte. */
extern u8 D_801448EB; /* @source 0x801448EB @kind data */

/* Shared main-RAM flag bank the mode-11 arm sets bit 0x91 of. */
extern u8 D_80144F28; /* @source 0x80144F28 @kind data */

/* Shared scenario progress byte the mode-11 and mode-12 arms gate on. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind data */

void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801C601C(u32 arg0);
void func_8015DF18(u16 arg0);
void func_8015B580(void* arg0, u8 bit_index);
void func_8015C058(void);

/* @source 0x801F4A0C
 * @behavior Steps the shared front-end mode byte 0x801448EC through this
 *           overlay's area-entry sequence, selecting the arm from the 13-slot
 *           jump table the payload keeps at 0x801F2C04; modes 0, 1, 10, 11 and
 *           12 have bodies and the remaining slots return untouched. Mode 0
 *           returns while the shared phase byte 0x80143BB0 still reads 2 and
 *           otherwise advances to mode 1. Mode 1 requests one front-selector
 *           context through 0x8019FA28 with 0xA7, 0xB0000, 0x698000 and kind
 *           0x89, then clears the shared state-entry tuple: byte 0x801448EB and
 *           the mode byte both go to zero. Mode 10 runs 0x801C601C(0) and
 *           advances to 11. Mode 11, once the scenario progress byte
 *           0x80146864 reads 1, queues cue 0x204 through 0x8015DF18, sets bit
 *           0x91 of the shared flag bank 0x80144F28 through 0x8015B580 and
 *           advances to 12; otherwise it returns untouched. Mode 12, once the
 *           scenario progress byte reads 2, runs the shared front-end step
 *           0x8015C058, clears that progress byte and clears the same
 *           state-entry tuple; otherwise it returns untouched. Takes no
 *           arguments and returns nothing.
 *
 * Name evidence: 0x801448EC is the shared mode byte and 0x801F2C04 the leading
 * table-data base the `dispatchSharedModeByteTable2C04*` family dispatches
 * through, and this body carries the family's own prologue (`lb` of the signed
 * byte, `sltiu` bound, `sll`/`lui %hi(jtbl)`/`addu`/`lw`/`jr`) with this
 * overlay's suffix and in-payload address. The payload names this body from no
 * table word and no `jal`: a whole-payload scan finds neither the address word
 * 0x801F4A0C nor a `jal` encoding that reaches it, so its caller is outside
 * this payload and every argument and callee role stays structural.
 *
 * Shape: the reviewed sibling
 * dispatchSharedModeByteTable2C04World04Area18813_801F4B80 supplies the switch
 * form, the signed-byte declaration and the placement route rather than a
 * clone: its 0x208-byte body carries eight arms this 0x120-byte span does not
 * have. The jump table is placed by this target's
 * `[[matching.section_placements]]` row for the `.rodata` section at
 * 0x801F2C04. The reset tail is reached from modes 1 and 12 only: the
 * original's empty table slots and its two guarded arms jump to the epilogue,
 * mode 1 jumps to the shared tail and mode 12 falls into it, so the label sits
 * inside the last arm and the arms that leave the shared tuple untouched
 * return. Mode 12 keeps the address of the progress byte in $s0 across the
 * 0x8015C058 call because it stores through it afterwards.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchSharedModeByteTable2C04World04Area16813_801F4A0C(void) {
  switch (D_801448EC) {
  case 0:
    if (D_80143BB0 == 2) {
      return;
    }
    D_801448EC = 1;
    return;
  case 1:
    func_8019FA28(0xA7, 0xB0000, 0x698000, 0x89);
    goto reset;
  case 10:
    func_801C601C(0);
    D_801448EC = 11;
    return;
  case 11:
    if (g_ScenarioProgress != 1) {
      return;
    }
    func_8015DF18(0x204);
    func_8015B580(&D_80144F28, 0x91);
    D_801448EC = 12;
    return;
  case 12:
    {
      u8* progress;

      progress = &g_ScenarioProgress;
      if (*progress != 2) {
        return;
      }
      func_8015C058();
      *progress = 0;
    }
  reset:
    D_801448EB = 0;
    D_801448EC = 0;
  }
}
